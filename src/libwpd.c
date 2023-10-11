// Windows USB I/O frontend for libwpd (https://github.com/petabyt/libwpd)
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>
#include <ptp.h>
#include <libwpd.h>

struct WpdStruct backend_wpd;

int ptp_comm_init(struct PtpRuntime *r) {
	ptp_generic_reset(r);
	wpd_init(1, L"Camlib WPD");

	return 0;	
}

int ptp_device_init(struct PtpRuntime *r) {
	ptp_comm_init();

	int length = 0;
	wchar_t **devices = wpd_get_devices(&backend_wpd, &length);

	if (length == 0) return PTP_NO_DEVICE;

	for (int i = 0; i < length; i++) {
		wprintf(L"Trying device: %s\n", devices[i]);

		int ret = wpd_open_device(&backend_wpd, devices[i]);
		if (ret) {
			return PTP_OPEN_FAIL;
		}

		int type = wpd_get_device_type(&backend_wpd);
		ptp_verbose_log("Found device of type: %d\n", type);
		if (type == WPD_DEVICE_TYPE_CAMERA) {
			return 0;
		}

		wpd_close_device(&backend_wpd);
	}

	return PTP_NO_DEVICE;
}

struct PtpDeviceEntry *ptpusb_device_list(struct PtpRuntime *r) {
	return NULL;
}

int ptp_device_open(struct PtpRuntime *r, struct PtpDeviceEntry *entry) {
	return PTP_IO_ERR;
}

int ptp_send_bulk_packets(struct PtpRuntime *r, int length) {
	struct PtpCommand cmd;
	struct PtpBulkContainer *bulk = (struct PtpBulkContainer*)(r->data);

	if (bulk->type == PTP_PACKET_TYPE_COMMAND) {
		cmd.code = bulk->code;
		cmd.param_length = ptp_get_param_length(r);
		for (int i = 0; i < cmd.param_length; i++) {
			cmd.params[i] = ptp_get_param(r, i);
		}
	
		int ret;
		if (r->data_phase_length) {
			ret = wpd_send_do_command(&backend_wpd, &cmd, r->data_phase_length);
			r->data_phase_length = 0;
		} else {
			ret = wpd_receive_do_command(&backend_wpd, &cmd);
		}

		if (ret) {
			return PTP_IO_ERR;
		} else {
			return length;
		}
	} else if (bulk->type == PTP_PACKET_TYPE_DATA) {
		cmd.param_length = 0;
		int ret = wpd_send_do_data(&backend_wpd, &cmd, ptp_get_payload(r), length - 12);
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
	// Don't do anything if the data phase was already sent
	if (r->data_phase_length) {
		r->data_phase_length = 0;
		return 12;
	}

	struct PtpBulkContainer *bulk = (struct PtpBulkContainer*)(r->data);
	if (bulk->type == PTP_PACKET_TYPE_COMMAND) {
		struct PtpCommand cmd;
		int b = wpd_receive_do_data(&backend_wpd, &cmd, (uint8_t *)(r->data + 12), 1024);
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
	}

	return 0;
}

int ptp_read_int(void *to, int length) {
	return 0;
}

int ptp_device_close(struct PtpRuntime *r) {
	wpd_close_device(&backend_wpd);
	return 0;
}

int ptp_device_reset(struct PtpRuntime *r) {
	return 0;
}
