// Easy to use operation (OC) functions. Requires backend.
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include <camlib.h>
#include <ptp.h>

int ptpip_init_command_request(struct PtpRuntime *r, char *device_name) {
	struct PtpIpInitPacket *p = (struct PtpIpInitPacket *)r->data;
	memset(p, 0, sizeof(struct PtpIpInitPacket));
	p->length = sizeof(struct PtpIpInitPacket);

	p->type = PTPIP_INIT_COMMAND_REQ;

	p->guid1 = 0xffffffff;
	p->guid2 = 0xffffffff;
	p->guid3 = 0xffffffff;
	p->guid4 = 0xffffffff;

	p->minor_ver = 1;

	ptp_write_unicode_string(p->device_name, "cam");

	if (ptpip_cmd_write(r, r->data, p->length) != p->length) return PTP_IO_ERR;

	// Read the packet size, then receive the rest
	int x = ptpip_cmd_read(r, r->data, 4);
	if (x < 0) return PTP_IO_ERR;
	x = ptpip_cmd_read(r, r->data + 4, p->length - 4);
	if (x < 0) return PTP_IO_ERR;

	// TODO: check for PTPIP_INIT_FAIL

	return 0;
}

// Technically not an OC, but fits snug here
int ptp_get_event(struct PtpRuntime *r, struct PtpEventContainer *ec) {
	int x = ptp_receive_int(r->data, r->max_packet_size);
	if (x < 0) {
		return x;
	} else {
		memcpy(ec, r->data, sizeof(struct PtpEventContainer));
	}

	return x;
}

int ptpip_init_events(struct PtpRuntime *r) {
	struct PtpIpHeader h;
	h.length = 12;
	h.type = PTPIP_INIT_EVENT_REQ;
	h.params[0] = 1;
	if (ptpip_event_send(r, &h, 12) != 12) {
		return PTP_IO_ERR;
	}

	if (ptpip_event_read(r, r->data, 8) != 8) {
		return PTP_IO_ERR;
	}

	return 0;
}

int ptp_custom_receive(struct PtpRuntime *r, int code) {
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

	if (ptp_receive_bulk_packets(r) < 0) return PTP_IO_ERR;
	return 0;
}

int ptp_close_session(struct PtpRuntime *r) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_CloseSession;
	cmd.param_length = 0;
	return ptp_generic_send(r, &cmd);
}

int ptp_get_device_info(struct PtpRuntime *r, struct PtpDeviceInfo *di) {
	// Assumes di is allocated - will be used during runtime later
	//r->di = di;

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
	cmd.params[1] = object_format;

	return ptp_generic_send(r, &cmd);
}

int ptp_init_open_capture(struct PtpRuntime *r, int storage_id, int object_format) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_InitiateOpenCapture;
	cmd.param_length = 2;
	cmd.params[0] = storage_id;
	cmd.params[1] = object_format;

	return ptp_generic_send(r, &cmd);
}

int ptp_terminate_open_capture(struct PtpRuntime *r, int trans) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_TerminateOpenCapture;
	cmd.param_length = 1;
	cmd.params[0] = trans;

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

int ptp_send_object_info(struct PtpRuntime *r, int storage_id, int handle, struct PtpObjectInfo *oi) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_SendObjectInfo;
	cmd.param_length = 2;
	cmd.params[0] = storage_id;
	cmd.params[1] = handle;

	char temp[1024];
	void *data = temp;
	int length = ptp_pack_object_info(r, oi, &data, sizeof(temp));
	if (length == 0) {
		return PTP_OUT_OF_MEM;
	}

	return ptp_generic_send_data(r, &cmd, temp, length);
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

// NOTE: raw JPEG contents is directly in the payload
int ptp_get_thumbnail(struct PtpRuntime *r, int handle) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_GetThumb;
	cmd.param_length = 1;
	cmd.params[0] = handle;

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

int ptp_set_prop_value(struct PtpRuntime *r, int code, int value) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_SetDevicePropValue;
	cmd.param_length = 1;
	cmd.params[0] = code;

	uint32_t dat[] = {value};

	return ptp_generic_send_data(r, &cmd, dat, sizeof(dat));
}

int ptp_set_prop_value_data(struct PtpRuntime *r, int code, void *data, int length) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_SetDevicePropValue;
	cmd.param_length = 1;
	cmd.params[0] = code;

	return ptp_generic_send_data(r, &cmd, data, length);
}

int ptp_delete_object(struct PtpRuntime *r, int handle, int format_code) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_DeleteObject;
	cmd.param_length = 2;
	cmd.params[0] = handle;
	cmd.params[1] = format_code;

	return ptp_generic_send(r, &cmd);	
}

int ptp_get_object(struct PtpRuntime *r, int handle) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_GetObject;
	cmd.param_length = 1;
	cmd.params[0] = handle;

	return ptp_generic_send(r, &cmd);	
}

int ptp_download_file(struct PtpRuntime *r, int handle, char *file) {
	int max = r->data_length - 12;

	struct PtpObjectInfo oi;
	if (ptp_get_object_info(r, handle, &oi)) {
		return 0;
	}

	max = oi.compressed_size;
	
	FILE *f = fopen(file, "w");
	if (f == NULL) {
		return PTP_RUNTIME_ERR;
	}

	int read = 0;
	while (1) {
		int x = ptp_get_partial_object(r, handle, read, max);
		if (x) {
			fclose(f);
			return x;
		}

		if (ptp_get_payload_length(r) == 0) {
			fclose(f);
			return 0;
		}

		fwrite(ptp_get_payload(r), 1, ptp_get_payload_length(r), f);

		read += ptp_get_payload_length(r);

		if (read == oi.compressed_size) {
			fclose(f);
			return 0;
		}
	}
}

int ptp_get_all_known(struct PtpRuntime *r, struct PtpGenericEvent **s, int *length) {
	uint16_t *props = r->di->props_supported;
	int plength = r->di->props_supported_length;
	*length = plength;

	*s = malloc(sizeof(struct PtpGenericEvent) * plength);

	for (int i = 0; i < plength; i++) {
		struct PtpGenericEvent *cur = &((*s)[i]);
		memset(cur, 0, sizeof(struct PtpGenericEvent));

		cur->code = props[i];

		int rc = ptp_get_prop_value(r, props[i]);
		if (rc) return rc;

		int v = ptp_parse_prop_value(r);
		cur->value = v;
		if (v == -1) {
			continue;
		}

		switch (props[i]) {
		case PTP_PC_BatteryLevel:
			cur->name = "battery";
			cur->value = v;
			break;
		}
	}

	return 0;
}
