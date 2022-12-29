// Generic misc things

#include <string.h>
#include <stdio.h>
#include <camlib.h>
#include <backend.h>
#include <ptp.h>

// May be slightly inneficient for every frame/action
// TODO: maybe 'cache' dev type for speed
int ptp_device_type(struct PtpRuntime *r) {
	struct PtpDeviceInfo *di = r->di;
	if (di == NULL) return PTP_DEV_EMPTY;
	if (!strcmp(di->manufacturer, "Canon Inc.")) {
		if (ptp_check_opcode(r, PTP_OC_EOS_GetStorageIDs)) {
			return PTP_DEV_EOS;
		}

		return PTP_DEV_CANON;
	} else if (!strcmp(di->manufacturer, "FUJIFILM")) {
		return PTP_DEV_FUJI;
	} else if (!strcmp(di->manufacturer, "Sony Corporation")) {
		return PTP_DEV_SONY;
	} else if (!strcmp(di->manufacturer, "Nikon Corporation")) {
		return PTP_DEV_NIKON;
	}

	return PTP_DEV_EMPTY;
}

int ptp_check_opcode(struct PtpRuntime *r, int op) {
	if (r->di == NULL) return 0;
	for (int i = 0; i < r->di->ops_supported_length; i++) {
		if (r->di->ops_supported[i] == op) {
			return 1;
		}
	}

	return 0;
}

int ptp_check_prop(struct PtpRuntime *r, int code) {
	if (r->di == NULL) return 0;
	for (int i = 0; i < r->di->props_supported_length; i++) {
		if (r->di->props_supported[i] == code) {
			return 1;
		}
	}

	return 0;
}

// Perform a "generic" command type transaction. Could be a macro, but macros suck
int ptp_generic_send(struct PtpRuntime *r, struct PtpCommand *cmd) {
	int length = ptp_new_cmd_packet(r, cmd);
	if (ptp_send_bulk_packets(r, length) != length) return PTP_IO_ERR;
	if (ptp_recieve_bulk_packets(r) < 0) return PTP_IO_ERR;

	if (ptp_get_return_code(r) == PTP_RC_OK) {
		return 0;
	} else {
		return PTP_CHECK_CODE;
	}
}

// Send a cmd packet, then data packet
int ptp_generic_send_data(struct PtpRuntime *r, struct PtpCommand *cmd, int length) {
	int plength = ptp_new_data_packet(r, cmd);
	ptp_update_data_length(r, plength + length);
	if (ptp_send_bulk_packets(r, length) != plength) return PTP_IO_ERR;
	if (ptp_recieve_bulk_packets(r) < 0) return PTP_IO_ERR;

	if (ptp_get_return_code(r) == PTP_RC_OK) {
		return 0;
	} else {
		return PTP_CHECK_CODE;
	}
}

int ptp_dump(struct PtpRuntime *r) {
	FILE *f = fopen("DUMP", "w");
	fwrite(r->data, r->data_length, 1, f);
	fclose(f);
	return 0;
}
