// Easy to use operation (OC) functions. Requires backend.
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)

#include <stddef.h>
#include <string.h>

#include <ptp.h>
#include <backend.h>
#include <camlib.h>

// Technically not an OC, but fits snug here
int ptp_get_event(struct PtpRuntime *r, struct PtpEventContainer *ec) {
	ptp_recieve_int(r->data, r->max_packet_size);
	memcpy(ec, r->data, sizeof(struct PtpEventContainer));
	return 0;
}

int ptp_custom_recieve(struct PtpRuntime *r, int code) {
	struct PtpCommand cmd;
	cmd.code = code;
	cmd.param_length = 0;

	return ptp_generic_send(r, &cmd);
}

int ptp_open_session(struct PtpRuntime *r) {
	r->session++;

	struct PtpCommand cmd;
	cmd.code = PTP_OC_OpenSession;
	cmd.params[0] = r->session;
	cmd.param_length = 1;
	cmd.data_length = 0;

	int length = ptp_new_cmd_packet(r, &cmd);

	// PTP open session transaction ID is always 0
	r->transaction = 0;

	if (ptp_send_bulk_packets(r, length) != length) return PTP_IO_ERR;

	// Set transaction ID back to start
	r->transaction = 1;

	if (ptp_recieve_bulk_packets(r) < 0) return PTP_IO_ERR;
	return 0;
}

int ptp_close_session(struct PtpRuntime *r) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_CloseSession;
	cmd.param_length = 0;
	return ptp_generic_send(r, &cmd);
}

int ptp_get_device_info(struct PtpRuntime *r, struct PtpDeviceInfo *di) {
	r->di = di;

	struct PtpCommand cmd;
	cmd.code = PTP_OC_GetDeviceInfo;
	cmd.param_length = 0;
	int x = ptp_generic_send(r, &cmd);
	if (x) {
		return x;
	} else {
		// Interfere with errors?
		return ptp_parse_device_info(r, di);
	}
}

int ptp_init_capture(struct PtpRuntime *r, int storage_id, int object_format) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_InitiateCapture;
	cmd.param_length = 2;
	cmd.params[0] = storage_id;
	cmd.params[0] = object_format;

	return ptp_generic_send(r, &cmd);
}

int ptp_get_storage_ids(struct PtpRuntime *r, struct UintArray **a) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_GetStorageIDs;
	cmd.param_length = 0;

	int x = ptp_generic_send(r, &cmd);
	*a = (void*)ptp_get_payload(r);
	return x;
}

int ptp_get_storage_info(struct PtpRuntime *r, int id, struct PtpStorageInfo *si) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_GetStorageInfo;
	cmd.param_length = 1;
	cmd.params[0] = id;

	int x = ptp_generic_send(r, &cmd);
	if (x) {
		return x;
	} else {
		memcpy(si, ptp_get_payload(r), sizeof(struct PtpStorageInfo));
		return 0;
	}
}

int ptp_get_object_info(struct PtpRuntime *r, uint32_t handle, struct PtpObjectInfo *oi) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_GetObjectInfo;
	cmd.param_length = 1;
	cmd.params[0] = handle;

	int x = ptp_generic_send(r, &cmd);
	if (x) {
		return x;
	} else {
		return ptp_parse_object_info(r, oi);
	}
}

int ptp_get_object_handles(struct PtpRuntime *r, int id, int format, int in, struct UintArray **a) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_GetObjectHandles;
	cmd.param_length = 3;
	cmd.params[0] = id;
	cmd.params[1] = format;
	cmd.params[2] = in;

	int x = ptp_generic_send(r, &cmd);
	*a = (void*)ptp_get_payload(r);
	return x;
}

int ptp_get_num_objects(struct PtpRuntime *r, int id, int format, int in) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_GetNumObjects;
	cmd.param_length = 3;
	cmd.params[0] = id;
	cmd.params[1] = format;
	cmd.params[2] = in;

	return ptp_generic_send(r, &cmd);
}

int ptp_get_prop_value(struct PtpRuntime *r, int code) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_GetDevicePropValue;
	cmd.param_length = 1;
	cmd.params[0] = code;
	return ptp_generic_send(r, &cmd);
}

int ptp_get_prop_desc(struct PtpRuntime *r, int code, struct PtpDevPropDesc *pd) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_GetDevicePropDesc;
	cmd.param_length = 1;
	cmd.params[0] = code;

	int x = ptp_generic_send(r, &cmd);

	// Return this?
	ptp_parse_prop_desc(r, pd);

	return x;
}

int ptp_get_thumbnail(struct PtpRuntime *r, int handle) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_GetThumb;
	cmd.param_length = 1;
	cmd.params[0] = handle;

	return ptp_generic_send(r, &cmd);
}

// Untested, nothing to test on (?)
int ptp_set_prop_value(struct PtpRuntime *r, int code, int value) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_GetDevicePropValue;
	cmd.param_length = 1;
	cmd.params[0] = code;

	int x = ptp_generic_send(r, &cmd);
	if (x) return x;

	// Send just value as the data packet
	uint32_t *t = (uint32_t*)ptp_get_payload(r);
	t[0] = (uint32_t)value;
	cmd.param_length = 0;
	ptp_generic_send_data(r, &cmd, 4);
	return x;
}

int ptp_delete_object(struct PtpRuntime *r, int handle, int format_code) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_DeleteObject;
	cmd.param_length = 2;
	cmd.params[0] = handle;
	cmd.params[1] = format_code;

	return ptp_generic_send(r, &cmd);	
}

int ptp_move_object(struct PtpRuntime *r, int handle, int storage_id, int parent_handle) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_MoveObject;
	cmd.param_length = 3;
	cmd.params[0] = handle;
	cmd.params[1] = storage_id;
	cmd.params[2] = parent_handle;

	return ptp_generic_send(r, &cmd);	
}
