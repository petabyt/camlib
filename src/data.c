// Parse/pack data and convert to (and from?) JSON
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>

int ptp_get_data_size(void *d, int type) {
	switch (type) {
	case PTP_TC_INT8:
	case PTP_TC_UINT8:
		return 1;
	case PTP_TC_INT16:
	case PTP_TC_UINT16:
		return 2;
	case PTP_TC_INT32:
	case PTP_TC_UINT32:
		return 4;
	case PTP_TC_INT64:
	case PTP_TC_UINT64:
		return 8;
	case PTP_TC_UINT8ARRAY:
	case PTP_TC_UINT16ARRAY:
	case PTP_TC_UINT32ARRAY:
	case PTP_TC_UINT64ARRAY:
		return ((uint32_t*)d)[0];
	case PTP_TC_STRING:
		return ((uint8_t*)d)[0];
	}

	return 0;
}

int ptp_parse_data(void **d, int type) {
	switch (type) {
	case PTP_TC_INT8:
	case PTP_TC_UINT8:
		return (uint8_t)ptp_read_uint8(d);
	case PTP_TC_INT16:
	case PTP_TC_UINT16:
		return (uint16_t)ptp_read_uint16(d);
	case PTP_TC_INT32:
	case PTP_TC_UINT32:
		return (uint32_t)ptp_read_uint32(d);
	}

	return ptp_get_data_size(*d, type);
}

int ptp_parse_prop_value(struct PtpRuntime *r) {
	void *d = ptp_get_payload(r);
	switch (ptp_get_payload_length(r)) {
	case 1:
		return (int)(((uint8_t *)d)[0]);
	case 2:
		return (int)(((uint16_t *)d)[0]);
	case 4:
		return (int)(((uint32_t *)d)[0]);
	}

	return -1;
}

int ptp_parse_prop_desc(struct PtpRuntime *r, struct PtpDevPropDesc *oi) {
	void *d = ptp_get_payload(r);
	memcpy(oi, d, PTP_PROP_DESC_VAR_START);
	d += PTP_PROP_DESC_VAR_START;
	oi->default_value = ptp_parse_data(&d, oi->data_type);
	oi->current_value = ptp_parse_data(&d, oi->data_type);

	// TODO: Form flag + form (for properties like date/time)
	return 0;
}

int ptp_parse_object_info(struct PtpRuntime *r, struct PtpObjectInfo *oi) {
	void *d = ptp_get_payload(r);
	memcpy(oi, d, PTP_OBJ_INFO_VAR_START);
	d += PTP_OBJ_INFO_VAR_START;
	ptp_read_string(&d, oi->filename, sizeof(oi->filename));
	ptp_read_string(&d, oi->date_created, sizeof(oi->date_created));
	ptp_read_string(&d, oi->date_modified, sizeof(oi->date_modified));
	ptp_read_string(&d, oi->keywords, sizeof(oi->keywords));

	return 0;
}

int ptp_pack_object_info(struct PtpRuntime *r, struct PtpObjectInfo *oi, void **d, int max) {
	if (1024 > max) {
		return 0;
	}

	void *b = *d;
	memcpy(*d, oi, PTP_OBJ_INFO_VAR_START);
	*d += PTP_OBJ_INFO_VAR_START;

	// If the string is empty, don't add it to the packet
	if (oi->filename[0] != '\0')
		ptp_write_string(d, oi->filename);
	if (oi->date_created[0] != '\0')
		ptp_write_string(d, oi->date_created);
	if (oi->date_modified[0] != '\0')
		ptp_write_string(d, oi->date_modified);
	if (oi->keywords[0] != '\0')
		ptp_write_string(d, oi->keywords);

	return (int)(*d - b);
}

int ptp_parse_device_info(struct PtpRuntime *r, struct PtpDeviceInfo *di) {
	// Skip packet header
	void *e = ptp_get_payload(r);

	di->standard_version = ptp_read_uint16(&e);
	di->vendor_ext_id = ptp_read_uint32(&e);
	di->version = ptp_read_uint16(&e);

	ptp_read_string(&e, di->extensions, sizeof(di->extensions));

	di->functional_mode = ptp_read_uint16(&e);

	di->ops_supported_length = ptp_read_uint16_array(&e, di->ops_supported, sizeof(di->ops_supported) / 2);
	di->events_supported_length = ptp_read_uint16_array(&e, di->events_supported, sizeof(di->events_supported) / 2);
	di->props_supported_length = ptp_read_uint16_array(&e, di->props_supported, sizeof(di->props_supported) / 2);
	di->capture_formats_length = ptp_read_uint16_array(&e, di->capture_formats, sizeof(di->capture_formats) / 2);
	di->playback_formats_length = ptp_read_uint16_array(&e, di->playback_formats, sizeof(di->playback_formats) / 2);

	ptp_read_string(&e, di->manufacturer, sizeof(di->manufacturer));
	ptp_read_string(&e, di->model, sizeof(di->model));

	ptp_read_string(&e, di->device_version, sizeof(di->device_version));
	ptp_read_string(&e, di->serial_number, sizeof(di->serial_number));

	r->di = di;

	return 0;
}

int ptp_device_info_json(struct PtpDeviceInfo *di, char *buffer, int max) {
	int curr = snprintf(buffer, max, "{\n    \"ops_supported\": [");
	for (int i = 0; i < di->ops_supported_length; i++) {
		char *end = ", ";
		if (i >= di->ops_supported_length - 1) {end = "";}
		curr += snprintf(buffer + curr, max - curr, "%d%s", di->ops_supported[i], end);
	}
	curr += snprintf(buffer + curr, max - curr, "],\n");

	curr += snprintf(buffer + curr, max - curr, "    \"events_supported\": [");
	for (int i = 0; i < di->events_supported_length; i++) {
		char *end = ", ";
		if (i >= di->events_supported_length - 1) {end = "";}
		curr += snprintf(buffer + curr, max - curr, "%d%s", di->events_supported[i], end);
	}
	curr += snprintf(buffer + curr, max - curr, "],\n");

	curr += snprintf(buffer + curr, max - curr, "    \"props_supported\": [");
	for (int i = 0; i < di->props_supported_length; i++) {
		char *end = ", ";
		if (i >= di->props_supported_length - 1) {end = "";}
		curr += snprintf(buffer + curr, max - curr, "%d%s", di->props_supported[i], end);
	}
	curr += snprintf(buffer + curr, max - curr, "],\n");

	curr += snprintf(buffer + curr, max - curr, "    \"manufacturer\": \"%s\",\n", di->manufacturer);
	curr += snprintf(buffer + curr, max - curr, "    \"extensions\": \"%s\",\n", di->extensions);
	curr += snprintf(buffer + curr, max - curr, "    \"model\": \"%s\",\n", di->model);
	curr += snprintf(buffer + curr, max - curr, "    \"device_version\": \"%s\",\n", di->device_version);
	curr += snprintf(buffer + curr, max - curr, "    \"serial_number\": \"%s\"\n", di->serial_number);
	curr += snprintf(buffer + curr, max - curr, "}");
	return curr;
}

const char *eval_obj_format(int code) {
	char *x = ptp_get_enum_all(code);
	switch (code) {
	case PTP_OF_Association:
		return "folder";
	default:
		if (x) return x;
		return "none";
	}
}

const char *eval_protection(int code) {
	switch (code) {
	case 0x0:
		return "none";
	case 0x1:
		return "read-only";
	case 0x8002:
		return "read-only data";
	case 0x8003:
		return "non-transferable data";
	default:
		return "reserved";
	}
}

int ptp_object_info_json(struct PtpObjectInfo *so, char *buffer, int max) {
	int curr = sprintf(buffer, "{");
	curr += snprintf(buffer + curr, max - curr, "\"storage_id\": %u,", so->storage_id);
	curr += snprintf(buffer + curr, max - curr, "\"compressed_size\": %u,", so->compressed_size);
	curr += snprintf(buffer + curr, max - curr, "\"parent\": %u,", so->parent_obj);
	curr += snprintf(buffer + curr, max - curr, "\"format\": \"%s\",", eval_obj_format(so->obj_format));
	curr += snprintf(buffer + curr, max - curr, "\"format_int\": %u,", so->obj_format);
	curr += snprintf(buffer + curr, max - curr, "\"protection\": \"%s\",", eval_protection(so->protection));
	curr += snprintf(buffer + curr, max - curr, "\"filename\": \"%s\",", so->filename);
	if (so->compressed_size != 0) {
		curr += snprintf(buffer + curr, max - curr, "\"img_width\": %u,", so->img_width);
		curr += snprintf(buffer + curr, max - curr, "\"img_height\": %u,", so->img_height);
	}
	curr += snprintf(buffer + curr, max - curr, "\"date_created\": \"%s\",", so->date_created);
	curr += snprintf(buffer + curr, max - curr, "\"date_modified\": \"%s\"", so->date_modified);
	curr += snprintf(buffer + curr, max - curr, "}");

	return curr;
}

const char *eval_storage_type(int id) {
	switch (id) {
	case 1:
		return "FixedROM";
	case 2:
		return "RemovableROM";
	case 3:
		return "Internal Storage";
	case 4:
		return "Removable Storage";
	default:
		return "Unknown";
	}
}

int ptp_storage_info_json(struct PtpStorageInfo *so, char *buffer, int max) {
	int len = sprintf(buffer, "{");
	len += snprintf(buffer + len, max - len, "\"storage_type\": \"%s\",", eval_storage_type(so->storage_type));
	len += snprintf(buffer + len, max - len, "\"fs_type\": %u,", so->fs_type);
	len += snprintf(buffer + len, max - len, "\"max_capacity\": %lu,", so->max_capacity);
	len += snprintf(buffer + len, max - len, "\"free_space\": %lu", so->free_space);
	len += snprintf(buffer + len, max - len, "}");
	return len;
}

int ptp_eos_prop_next(void **d, struct PtpGenericEvent *p) {
	uint32_t code = ptp_read_uint32(d);
	uint32_t value = ptp_read_uint32(d);

	const char *name = ptp_get_enum_all(code);
	const char *str_value = NULL;
	switch (code) {
	case PTP_PC_EOS_Aperture:
		value = ptp_eos_get_aperture(value, 0);
		name = "aperture";
		break;
	case PTP_PC_EOS_ShutterSpeed:
		value = ptp_eos_get_shutter(value, 0);
		name = "shutter speed";
		break;
	case PTP_PC_EOS_ISOSpeed:
		value = ptp_eos_get_iso(value, 0);
		name = "iso";
		break;
	case PTP_PC_EOS_BatteryPower:
		// EOS has 3 battery bars
		value++;
		if (value == 3) value = 4;
		name = "battery";
		break;
	case PTP_PC_EOS_ImageFormat: {
			int data[5] = {value, ptp_read_uint32(d), ptp_read_uint32(d),
				ptp_read_uint32(d), ptp_read_uint32(d)};
			if (value == 1) {
				value = ptp_eos_get_imgformat_value(data);
			} else {
				value = IMG_FORMAT_RAW_JPEG;
			}
			name = "image format";
		} break;
	case PTP_PC_EOS_VF_Output:
		name = "mirror";
		if (value == 0) {
			str_value = "finder";
		} else {
			str_value = "open";
		}
		break;
	case PTP_PC_EOS_AEModeDial:
		name = "mode dial";
		break;
	case PTP_PC_EOS_FocusMode:
		name = "focus mode";
		if (value == 3) {
			str_value = "MF";
		} else {
			str_value = "AF";
		}
		break;
	case PTP_PC_EOS_WhiteBalance:
		name = "white balance";
		value = ptp_eos_get_white_balance(value, 0);
		break;
	case PTP_PC_EOS_FocusInfoEx:
		name = "focused";
		break;
	}

	p->code = code;
	if (name == enum_null) {
		p->name = NULL;
	} else {
		p->name = name;
	}

	p->value = value;
	if (str_value == NULL) {
		p->str_value = NULL;
	} else {
		p->str_value = str_value;
	}

	return 0;
}

int ptp_eos_prop_json(void **d, char *buffer, int max) {
	struct PtpGenericEvent p;

	ptp_eos_prop_next(d, &p);

	int curr = 0;
	if (p.name == NULL) {
		curr = snprintf(buffer + curr, max - curr, "[%u, %u]\n", p.code, p.value);
	} else {
		if (p.str_value == NULL) {
			curr = snprintf(buffer + curr, max - curr, "[\"%s\", %u]\n", p.name, p.value);
		} else {
			curr = snprintf(buffer + curr, max - curr, "[\"%s\", \"%s\"]\n", p.name, p.str_value);
		}
	}

	return curr;
}

int ptp_eos_events(struct PtpRuntime *r, struct PtpGenericEvent **p) {
	//struct PtpCanonEvent ce;
	void *dp = ptp_get_payload(r);

	int length = 0;
	while (dp != NULL) {
		if (dp >= (void*)ptp_get_payload(r) + ptp_get_data_length(r)) break;
		void *d = dp;
		uint32_t size = ptp_read_uint32(&d);
		uint32_t type = ptp_read_uint32(&d);

		dp += size;

		// TODO: length is 1 when props list is invalid/empty
		if (type == 0) break;

		length++;
	}

	if (length == 0) return 0;

	(*p) = malloc(sizeof(struct PtpGenericEvent) * length);

	dp = ptp_get_payload(r);
	for (int i = 0; i < length; i++) {
		struct PtpGenericEvent *cur = &((*p)[i]);
		memset(cur, 0, sizeof(struct PtpGenericEvent));

		// Read header
		void *d = dp;
		uint32_t size = ptp_read_uint32(&d);
		uint32_t type = ptp_read_uint32(&d);

		// Detect termination or overflow
		if (type == 0) break;

		switch (type) {
		case PTP_EC_EOS_PropValueChanged:
			ptp_eos_prop_next(&d, cur);
			break;
		case PTP_EC_EOS_InfoCheckComplete:
		case PTP_PC_EOS_FocusInfoEx:
			cur->name = ptp_get_enum_all(type);
			break;
		case PTP_EC_EOS_RequestObjectTransfer: {
			int a = ptp_read_uint32(&d);
			int b = ptp_read_uint32(&d);
			cur->name = "request object transfer";
			cur->code = a;
			cur->value = b;
			} break;
		case PTP_EC_EOS_ObjectAddedEx: {
			struct PtpEOSObject *obj = (struct PtpEOSObject *)d;
			cur->name = "new object";
			cur->value = obj->a;
			} break;
		}

		// Move dp over for the next entry
		dp += size;
	}

	return length;
}

int ptp_eos_events_json(struct PtpRuntime *r, char *buffer, int max) {
	void *dp = ptp_get_payload(r);

	int curr = sprintf(buffer, "[\n");

	int tmp = 0;
	while (dp != NULL) {
		void *d = dp;
		uint32_t size = ptp_read_uint32(&d);
		uint32_t type = ptp_read_uint32(&d);

		dp += size;

		if (type == 0) break;
		if (dp >= (void*)ptp_get_payload(r) + ptp_get_data_length(r)) break;

		// Don't put comma for last entry
		char *end = "";
		if (tmp) end = ",";
		tmp++;

		curr += sprintf(buffer + curr, "%s", end);

		switch (type) {
		case PTP_EC_EOS_PropValueChanged:
			curr += ptp_eos_prop_json(&d, buffer + curr, max - curr);
			break;
		case PTP_EC_EOS_InfoCheckComplete:
		case PTP_PC_EOS_FocusInfoEx:
			curr += sprintf(buffer + curr, "[\"%s\", %u]\n", ptp_get_enum_all(type), type);
			break;
		case PTP_EC_EOS_RequestObjectTransfer: {
			int a = ptp_read_uint32(&d);
			int b = ptp_read_uint32(&d);
			curr += sprintf(buffer + curr, "[%u, %u]\n", a, b);
			} break;
		case PTP_EC_EOS_ObjectAddedEx: {
			struct PtpEOSObject *obj = (struct PtpEOSObject *)d;
			curr += sprintf(buffer + curr, "[\"new object\", %u]\n", obj->a);
			} break;
		default:
			// Unknown event, delete the comma
			curr -= strlen(end);
			if (tmp == 1) tmp = 0;
		}

		if (curr >= max) return 0;
	}

	curr += sprintf(buffer + curr, "]");
	return curr;
}

// TODO: Rename to ptp_fuji_get_init_info
int ptp_fuji_get_init_info(struct PtpRuntime *r, struct PtpFujiInitResp *resp) {
	void *dat = ptp_get_payload(r);

	resp->x1 = ptp_read_uint32(&dat);
	resp->x2 = ptp_read_uint32(&dat);
	resp->x3 = ptp_read_uint32(&dat);
	resp->x4 = ptp_read_uint32(&dat);

	ptp_read_unicode_string(resp->cam_name, (char *)(ptp_get_payload(r) + 16), sizeof(resp->cam_name));

	return 0;
}

int ptp_fuji_parse_object_info(struct PtpRuntime *r, struct PtpFujiObjectInfo *oi) {
	void *d = ptp_get_payload(r);
	memcpy(oi, d, PTP_FUJI_OBJ_INFO_VAR_START);
	d += PTP_FUJI_OBJ_INFO_VAR_START;
	ptp_read_string(&d, oi->filename, sizeof(oi->filename));

	/* TODO: Figure out payload later:
		0D 44 00 53 00 43 00 46 00 35 00 30 00 38 00 37 00 2E 00 4A 00 50 00 47 00
		00 00 10 32 00 30 00 31 00 35 00 30 00 35 00 32 00 34 00 54 00 30 00 31 00
		31 00 37 00 31 00 30 00 00 00 00 0E 4F 00 72 00 69 00 65 00 6E 00 74 00 61
		00 74 00 69 00 6F 00 6E 00 3A 00 31 00 00 00 0C 00 00 00 03 00 01 20 0E 00
		00 00 00
	*/

	return 0;
}
