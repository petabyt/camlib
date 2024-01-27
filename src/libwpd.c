// Windows USB I/O frontend for libwpd (https://github.com/petabyt/libwpd)
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>
#include <ptp.h>
#include <libwpd.h>

#define WPD_VERBOSE 1

int ptp_comm_init(struct PtpRuntime *r) {
	ptp_reset(r);

	// We are not using low-level I/O operations, so this is never used
	r->max_packet_size = 512;

	// in libwpd, this will only coinitalize the new thread. Orig thread may be dead.
	wpd_init(WPD_VERBOSE, L"Camlib WPD");

	if (r->comm_backend != NULL) return 0;

	r->comm_backend = malloc(sizeof(struct WpdStruct));

	return 0;	
}

int ptp_device_init(struct PtpRuntime *r) {
	if (!r->io_kill_switch) {
		ptp_verbose_log("Connection is active\n");
		return NULL;
	}

	ptp_comm_init(r);

	struct WpdStruct *wpd = (struct WpdStruct *)(r->comm_backend);
	if (wpd == NULL) return PTP_IO_ERR;

	ptp_mutex_lock(r);

	int length = 0;
	wchar_t **devices = wpd_get_devices(wpd, &length);

	if (length == 0) {
		ptp_mutex_unlock(r);
		return PTP_NO_DEVICE;
	}

	for (int i = 0; i < length; i++) {
		wprintf(L"Trying device: %s\n", devices[i]);

		int ret = wpd_open_device(wpd, devices[i]);
		if (ret) {
			ptp_mutex_unlock(r);
			return PTP_OPEN_FAIL;
		}

		int type = wpd_get_device_type(wpd);
		ptp_verbose_log("Found device of type: %d\n", type);
		if (type == WPD_DEVICE_TYPE_CAMERA) {
			r->io_kill_switch = 0;
			ptp_mutex_unlock(r);
			return 0;
		}

		wpd_close_device(wpd);
	}

	ptp_mutex_unlock(r);
	return PTP_NO_DEVICE;
}

// Unimplemented, don't use yet
#if 0
struct PtpDeviceEntry *ptpusb_device_list(struct PtpRuntime *r) {
	return NULL;
}

int ptp_device_open(struct PtpRuntime *r, struct PtpDeviceEntry *entry) {
	// Unimplemented
	return PTP_IO_ERR;
}
#endif

int ptp_send_bulk_packets(struct PtpRuntime *r, int length) {
	if (r->io_kill_switch) return PTP_IO_ERR;

	struct WpdStruct *wpd = (struct WpdStruct *)(r->comm_backend);
	if (wpd == NULL) return PTP_IO_ERR;

	struct LibWPDPtpCommand cmd;
	struct PtpBulkContainer *bulk = (struct PtpBulkContainer*)(r->data);

	if (bulk->code == PTP_OC_OpenSession || bulk->code == PTP_OC_CloseSession) {
		return length;
	}

	if (bulk->type == PTP_PACKET_TYPE_COMMAND) {
		cmd.code = bulk->code;
		cmd.param_length = ptp_get_param_length(r);
		for (int i = 0; i < cmd.param_length; i++) {
			cmd.params[i] = ptp_get_param(r, i);
		}
	
		int ret;
		if (r->data_phase_length) {
			ret = wpd_send_do_command(wpd, &cmd, r->data_phase_length);
			r->data_phase_length = 0;
		} else {
			ret = wpd_receive_do_command(wpd, &cmd);
		}

		if (ret) {
			return PTP_IO_ERR;
		} else {
			return length;
		}
	} else if (bulk->type == PTP_PACKET_TYPE_DATA) {
		cmd.param_length = 0;
		int ret = wpd_send_do_data(wpd, &cmd, ptp_get_payload(r), length - 12);
		if (ret < 0) {
			return PTP_IO_ERR;
		} else {
			// Signal for ptp_receive_bulk_packets, finished command already sent
			r->data_phase_length = ret;

			bulk->length = 12;
			bulk->type = PTP_PACKET_TYPE_RESPONSE;
			bulk->code = cmd.code;
			bulk->transaction = r->transaction + 1;
			
			return length;
		}
	} else {
		return PTP_IO_ERR;
	}

	return 0;
}

int ptp_receive_bulk_packets(struct PtpRuntime *r) {
	if (r->io_kill_switch) return PTP_IO_ERR;

	// Don't do anything if the data phase was already sent
	if (r->data_phase_length) {
		r->data_phase_length = 0;
		return 12;
	}

	struct WpdStruct *wpd = (struct WpdStruct *)(r->comm_backend);
	if (wpd == NULL) return PTP_IO_ERR;

	// WPD doesn't let you send session opcodes
	struct PtpBulkContainer *bulk = (struct PtpBulkContainer*)(r->data);
	if (bulk->code == PTP_OC_OpenSession || bulk->code == PTP_OC_CloseSession) {
		bulk->length = 12;
		bulk->code = PTP_RC_OK;
		bulk->type = PTP_PACKET_TYPE_RESPONSE;
		memset(bulk->params, 0, 5 * sizeof(uint32_t));
		return 12;
	}

	struct LibWPDPtpCommand cmd;
	int b = wpd_receive_do_data(wpd, &cmd, (uint8_t *)(r->data + 12), r->data_length - 12);
	if (b < 0) {
		return PTP_IO_ERR;
	}
	
	if (b == 0) {
		bulk->length = 12;
		bulk->type = PTP_PACKET_TYPE_RESPONSE;
		bulk->code = cmd.code;
		bulk->transaction = r->transaction + 1;
	} else {
		bulk->length = 12 + b;
		bulk->type = PTP_PACKET_TYPE_DATA;
		bulk->code = cmd.code;
		bulk->transaction = r->transaction + 1;

		struct PtpBulkContainer *resp = (struct PtpBulkContainer*)(r->data + 12 + b);
		resp->length = 12;
		resp->type = PTP_PACKET_TYPE_RESPONSE;
		resp->code = cmd.code;
		resp->transaction = r->transaction + 1;
	}

	return 0;
}

int ptp_read_int(struct PtpRuntime *r, void *to, int length) {
	return 0;
}

int ptp_device_close(struct PtpRuntime *r) {
	struct WpdStruct *wpd = (struct WpdStruct *)(r->comm_backend);
	if (wpd == NULL) return PTP_IO_ERR;
	wpd_close_device(wpd);
	return 0;
}

int ptp_device_reset(struct PtpRuntime *r) {
	return 0;
}
