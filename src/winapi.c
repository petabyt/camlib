#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>
#include <backend.h>
#include <ptp.h>
#include <operations.h>
#include <winapi.h>

struct WpdStruct backend_wpd;

#if 0
int wpdfoo() {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_EOS_SetDevicePropValueEx;
	cmd.param_length = 0;
	int ret = wpd_send_do_command(&backend_wpd, &cmd, 12);
	puts("Send do command");
	if (ret) {
		printf("IO error, %d\n", ret);
		return 1;
	}

	uint32_t *data = malloc(12);
	data[0] = 0xc;
	data[1] = PTP_PC_EOS_ISOSpeed;
	data[2] = ptp_eos_get_iso(6400, 1);
	ret = wpd_send_do_data(&backend_wpd, &cmd, data, 12);
	if (ret) {
		printf("IO error, %d\n", ret);
	}
}
#endif

int ptp_device_init(struct PtpRuntime *r) {
	wpd_init(1, L"Camlib WPD");
	int length = 0;
	wchar_t **devices = wpd_get_devices(&backend_wpd, &length);

	if (length == 0) return PTP_NO_DEVICE;

	for (int i = 0; i < length; i++) {
		wprintf(L"Device: %s\n", devices[i]);
	}

	int ret = wpd_open_device(&backend_wpd, devices[0]);
	if (ret) {
		return PTP_OPEN_FAIL;
	}

	//free(devices);

	return 0;
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
			ret = wpd_recieve_do_command(&backend_wpd, &cmd);
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
			// Signal for ptp_recieve_bulk_packets, finished command already sent
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

int ptp_recieve_bulk_packets(struct PtpRuntime *r) {
	// Don't do anything if the data phase was already sent
	if (r->data_phase_length) {
		r->data_phase_length = 0;
		return 12;
	}

	struct PtpBulkContainer *bulk = (struct PtpBulkContainer*)(r->data);
	if (bulk->type == PTP_PACKET_TYPE_COMMAND) {
		struct PtpCommand cmd;
		int b = wpd_recieve_do_data(&backend_wpd, &cmd, (uint8_t *)(r->data + 12), 1024);
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

int ptp_recieve_int(void *to, int length) {
	return PTP_IO_ERR;
}

int ptp_device_close(struct PtpRuntime *r) {
	wpd_close_device(&backend_wpd);
	return 0;
}
