// Easy to use operation (OC) functions. Requires backend.
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)

#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include <ptp.h>
#include <ptpbackend.h>
#include <camlib.h>

// Technically not an OC, but fits snug here
int ptp_get_event(struct PtpRuntime *r, struct PtpEventContainer *ec) {
	int x = ptp_recieve_int(r->data, r->max_packet_size);
	if (x < 0) {
		return x;
	} else {
		memcpy(ec, r->data, sizeof(struct PtpEventContainer));
	}

	return x;
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

int ptp_get_partial_object(struct PtpRuntime *r, uint32_t handle, int offset, int max) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_GetPartialObject;
	cmd.param_length = 3;
	cmd.params[0] = handle;
	cmd.params[1] = offset;
	cmd.params[2] = max;

	// What was the **ptr for?

	int x = ptp_generic_send(r, &cmd);
	return x;
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

// raw JPEG contents is in the payload
int ptp_get_thumbnail(struct PtpRuntime *r, int handle) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_GetThumb;
	cmd.param_length = 2;
	cmd.params[0] = handle; // -1 to delete all image
	cmd.params[1] = 0; // Object format code

	return ptp_generic_send(r, &cmd);
}

int ptp_move_object(struct PtpRuntime *r, int storage_id, int handle, int folder) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_GetThumb;
	cmd.param_length = 3;
	cmd.params[0] = handle;
	cmd.params[1] = storage_id;
	cmd.params[2] = folder;

	return ptp_generic_send(r, &cmd);
}

// Untested, nothing to test on (?)
int ptp_set_prop_value(struct PtpRuntime *r, int code, int value) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_GetDevicePropValue;
	cmd.param_length = 1;
	cmd.params[0] = code;

	uint32_t dat[] = {value};

	return ptp_generic_send_data(r, &cmd, dat, sizeof(dat));
}

int ptp_delete_object(struct PtpRuntime *r, int handle, int format_code) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_DeleteObject;
	cmd.param_length = 2;
	cmd.params[0] = handle;
	cmd.params[1] = format_code;

	return ptp_generic_send(r, &cmd);	
}

int ptp_download_file(struct PtpRuntime *r, int handle, char *file) {
	int max = r->data_length - (r->max_packet_size * 2);
	printf("%d\n", max);

	FILE *f = fopen(file, "w");
	if (f == NULL) {
		return PTP_RUNTIME_ERR;
	}

	int read = 0;
	while (1) {
		int x = ptp_get_partial_object(r, handle, read, max);
		if (x) {
			return x;
		}

		fwrite(ptp_get_payload(r), 1, ptp_get_payload_length(r), f);
		
		if (ptp_get_payload_length(r) < max) {
			return read;
		}

		read += ptp_get_payload_length(r);
	}
}
