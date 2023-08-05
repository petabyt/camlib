// Basic implementation of Fujifilm WiFi and USB functions
// Copyright 2023 by Daniel C (https://github.com/petabyt/camlib)

#include <string.h>
#include <camlib.h>
#include <ptp.h>

#define FUJI_PROTOCOL_VERSION 0x8f53e4f2

// TODO: 902b device info implementation

// PTP vendor version of SendObjectInfo (USB & IP)
int ptp_fuji_send_object_info(struct PtpRuntime *r, struct PtpObjectInfo *oi) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_FUJI_SendObjectInfo;
	cmd.param_length = 0;

	char temp[1024];
	void *data = temp;
	int length = ptp_pack_object_info(r, oi, &data, sizeof(temp));
	if (length == 0) {
		return PTP_OUT_OF_MEM;
	}

	return ptp_generic_send_data(r, &cmd, temp, length);
}

// PTP vendor version of SendObject (USB & IP)
int ptp_fuji_send_object(struct PtpRuntime *r, struct PtpObjectInfo *oi, void *data, int length) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_FUJI_SendObject;
	cmd.param_length = 0;

	return ptp_generic_send_data(r, &cmd, data, length);
}

int ptpip_fuji_init(struct PtpRuntime *r, char *device_name) {
	struct FujiInitPacket *p = (struct FujiInitPacket *)r->data;
	memset(p, 0, sizeof(struct FujiInitPacket));
	p->length = 0x52;
	p->type = PTPIP_INIT_COMMAND_REQ;

	p->version = FUJI_PROTOCOL_VERSION;

	p->guid1 = 0x5d48a5ad;
	p->guid2 = 0xb7fb287;
	p->guid3 = 0xd0ded5d3;
	p->guid4 = 0x0;

	ptp_write_unicode_string(p->device_name, device_name);

	if (ptpip_cmd_write(r, r->data, p->length) != p->length) return PTP_IO_ERR;

	// Read the packet size, then recieve the rest
	int x = ptpip_cmd_read(r, r->data, 4);
	if (x < 0) return PTP_IO_ERR;
	x = ptpip_cmd_read(r, r->data + 4, p->length - 4);
	if (x < 0) return PTP_IO_ERR;

	if (ptp_get_return_code(r) == 0x0) {
		return 0;
	} else {
		return PTP_IO_ERR;
	}
}

int ptpip_fuji_get_events(struct PtpRuntime *r) {
	int rc = ptp_get_prop_value(r, PTP_PC_FUJI_EventsList);
	if (rc) return rc;

	struct PtpFujiEvents *ev = (struct PtpFujiEvents *)(ptp_get_payload(r));
	ptp_verbose_log("Found %d events\n", ev->length);
	for (int i = 0; i < ev->length; i++) {
		ptp_verbose_log("%X changed to %d\n", ev->events[i].code, ev->events[i].value);
	}
	return 0;
}

int ptpip_fuji_wait_unlocked(struct PtpRuntime *r) {
	int rc = ptp_get_prop_value(r, PTP_PC_FUJI_CameraState);
	if (rc) {
		return rc;
	}

	// If PTP_PC_FUJI_Unlocked is non-zero, that means it doesn't need to be polled
	int value = ptp_parse_prop_value(r);
	if (value != 0) {
		// Set the value back to let the camera know the software supports it
		rc = ptp_set_prop_value(r, PTP_PC_FUJI_CameraState, value);
		return rc;
	}

	while (1) {
		rc = ptp_get_prop_value(r, PTP_PC_FUJI_EventsList);
		if (rc) {
			return rc;
		}

		// Apply events structure to payload, and check for unlocked event (PTP_PC_FUJI_Unlocked)
		struct PtpFujiEvents *ev = (struct PtpFujiEvents *)(ptp_get_payload(r));
		for (int i = 0; i < ev->length; i++) {
			if (ev->events[i].code == PTP_PC_FUJI_CameraState && (ev->events[i].value != 0x0)) {
				return 0;
			}
		}

		CAMLIB_SLEEP(100);
	}
}

int ptpip_fuji_get_object_info(struct PtpRuntime *r, uint32_t handle, struct PtpFujiObjectInfo *oi) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_GetObjectInfo;
	cmd.param_length = 1;
	cmd.params[0] = handle;

	int x = ptp_generic_send(r, &cmd);
	if (x) {
		return x;
	} else {
		return ptp_fuji_parse_object_info(r, oi);
	}
}
