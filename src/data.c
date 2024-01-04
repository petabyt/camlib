// Data structure unpacking and packing functions
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)
// This code uses unaligned access/writes, will probably be changed in the future

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <camlib.h>

// Custom snprint with offset - for safer string building
// Eventually, this should be used for all JSON string builders
static int osnprintf(char *str, int cur, int size, const char *format, ...) {
	if (size - cur < 0) {
		return 0;
	}

	int r;
	va_list args;
	va_start(args, format);
	r = vsnprintf(str + cur, size - cur, format, args);
	va_end(args);

	return r;
}

// We do not have proper UTF16 support for now
static void format_sane_string(char *string) {
	for (int i = 0; string[i] != '\0'; i++) {
		if (string[i] < 0) {
			string[i] = '?';
		} else if (string[i] < 32) {
			string[i] = ' ';
		}
	}
}

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
	uint8_t *d = ptp_get_payload(r);
	memcpy(oi, d, PTP_PROP_DESC_VAR_START);
	d += PTP_PROP_DESC_VAR_START;
	oi->default_value = ptp_parse_data((void **)&d, oi->data_type);
	oi->current_value = ptp_parse_data((void **)&d, oi->data_type);

	// TODO: Form flag + form (for properties like date/time)
	return 0;
}

int ptp_parse_object_info(struct PtpRuntime *r, struct PtpObjectInfo *oi) {
	uint8_t *d = ptp_get_payload(r);
	memcpy(oi, d, PTP_OBJ_INFO_VAR_START);
	d += PTP_OBJ_INFO_VAR_START;
	ptp_read_string((void **)&d, oi->filename, sizeof(oi->filename));
	ptp_read_string((void **)&d, oi->date_created, sizeof(oi->date_created));
	ptp_read_string((void **)&d, oi->date_modified, sizeof(oi->date_modified));
	ptp_read_string((void **)&d, oi->keywords, sizeof(oi->keywords));

	return 0;
}

// TODO: Different API
int ptp_pack_object_info(struct PtpRuntime *r, struct PtpObjectInfo *oi, void **dat, int max) {
	if (1024 > max) {
		return 0;
	}

	uint8_t **ptr = (uint8_t **)(dat);
	memcpy(*ptr, oi, PTP_OBJ_INFO_VAR_START);
	(*ptr) += PTP_OBJ_INFO_VAR_START;

	int length = PTP_OBJ_INFO_VAR_START;

	// If the string is empty, don't add it to the packet
	if (oi->filename[0] != '\0')
		length += ptp_write_string((void **)(ptr), oi->filename);
	if (oi->date_created[0] != '\0')
		length += ptp_write_string((void **)(ptr), oi->date_created);
	if (oi->date_modified[0] != '\0')
		length += ptp_write_string((void **)(ptr), oi->date_modified);
	if (oi->keywords[0] != '\0')
		length += ptp_write_string((void **)(ptr), oi->keywords);

	// Return pointer length added
	return length;
}

void *ptp_pack_chdk_upload_file(struct PtpRuntime *r, char *in, char *out, int *length) {
	FILE *f = fopen(in, "rb");
	if (f == NULL) {
		ptp_verbose_log("Unable to open %s\n", in);
		return NULL;
	}

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

	int size_all = 4 + strlen(out) + 1 + file_size;
	*length = size_all;
	char *data = malloc(size_all);
	if (data == NULL) return NULL;

	void *d_ptr = (void *)data;
	ptp_write_uint32(&d_ptr, strlen(out) + 1);
	ptp_write_utf8_string(&d_ptr, out);

	fread(d_ptr, 1, file_size, f);

	return data;
}

int ptp_parse_device_info(struct PtpRuntime *r, struct PtpDeviceInfo *di) {
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

	format_sane_string(di->manufacturer);
	format_sane_string(di->model);
	format_sane_string(di->device_version);
	format_sane_string(di->serial_number);

	r->di = di;

	return 0;
}

int ptp_device_info_json(struct PtpDeviceInfo *di, char *buffer, int max) {
	int curr = osnprintf(buffer, 0, max, "{\n    \"opsSupported\": [");
	for (int i = 0; i < di->ops_supported_length; i++) {
		char *end = ", ";
		if (i >= di->ops_supported_length - 1) end = "";
		curr += osnprintf(buffer, curr, max, "%d%s", di->ops_supported[i], end);
	}
	curr += osnprintf(buffer, curr, max, "],\n");

	curr += osnprintf(buffer, curr, max, "    \"eventsSupported\": [");
	for (int i = 0; i < di->events_supported_length; i++) {
		char *end = ", ";
		if (i >= di->events_supported_length - 1) end = "";
		curr += osnprintf(buffer, curr, max, "%d%s", di->events_supported[i], end);
	}
	curr += osnprintf(buffer, curr, max, "],\n");

	curr += osnprintf(buffer, curr, max, "    \"propsSupported\": [");
	for (int i = 0; i < di->props_supported_length; i++) {
		char *end = ", ";
		if (i >= di->props_supported_length - 1) end = "";
		curr += osnprintf(buffer, curr, max, "%d%s", di->props_supported[i], end);
	}
	curr += osnprintf(buffer, curr, max, "],\n");

	curr += osnprintf(buffer, curr, max, "    \"manufacturer\": \"%s\",\n", di->manufacturer);
	curr += osnprintf(buffer, curr, max, "    \"extensions\": \"%s\",\n", di->extensions);
	curr += osnprintf(buffer, curr, max, "    \"model\": \"%s\",\n", di->model);
	curr += osnprintf(buffer, curr, max, "    \"deviceVersion\": \"%s\",\n", di->device_version);
	curr += osnprintf(buffer, curr, max, "    \"serialNumber\": \"%s\"\n", di->serial_number);
	curr += osnprintf(buffer, curr, max, "}");
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
	curr += snprintf(buffer + curr, max - curr, "\"compressedSize\": %u,", so->compressed_size);
	curr += snprintf(buffer + curr, max - curr, "\"parent\": %u,", so->parent_obj);
	curr += snprintf(buffer + curr, max - curr, "\"format\": \"%s\",", eval_obj_format(so->obj_format));
	curr += snprintf(buffer + curr, max - curr, "\"format_int\": %u,", so->obj_format);
	curr += snprintf(buffer + curr, max - curr, "\"protection\": \"%s\",", eval_protection(so->protection));
	curr += snprintf(buffer + curr, max - curr, "\"filename\": \"%s\",", so->filename);
	if (so->compressed_size != 0) {
		curr += snprintf(buffer + curr, max - curr, "\"imgWidth\": %u,", so->img_width);
		curr += snprintf(buffer + curr, max - curr, "\"imgHeight\": %u,", so->img_height);
	}
	curr += snprintf(buffer + curr, max - curr, "\"dateCreated\": \"%s\",", so->date_created);
	curr += snprintf(buffer + curr, max - curr, "\"dateModified\": \"%s\"", so->date_modified);
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
	int len = osnprintf(buffer, 0, max, "{");
	len += osnprintf(buffer, len, max, "\"storageType\": \"%s\",", eval_storage_type(so->storage_type));
	len += osnprintf(buffer, len, max, "\"fsType\": %u,", so->fs_type);
	len += osnprintf(buffer, len, max, "\"maxCapacity\": %lu,", so->max_capacity);
	len += osnprintf(buffer, len, max, "\"freeSpace\": %lu", so->free_space);
	len += osnprintf(buffer, len, max, "}");
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

// TODO: Stream reader for events
struct PtpEventReader {
	int entries;
	int index;
	void *ptr;
};

int ptp_eos_events_length(struct PtpRuntime *r) {
	uint8_t *dp = ptp_get_payload(r);

	int length = 0;
	while (dp != NULL) {
		if (dp >= (uint8_t *)ptp_get_payload(r) + ptp_get_payload_length(r)) break;
		void *d = dp;
		uint32_t size = ptp_read_uint32((void **)&d);
		uint32_t type = ptp_read_uint32((void **)&d);

		dp += size;

		// TODO: length is 1 when props list is invalid/empty
		if (type == 0) break;

		length++;
	}

	return length;
}

// TODO: misnomer: ptp_eos_unpack_events
// TODO: we should have a way to read next entry without allocating/freeing memory
int ptp_eos_events(struct PtpRuntime *r, struct PtpGenericEvent **p) {
	int length = ptp_eos_events_length(r);

	if (length == 0) return 0;
	if (length < 0) return length;

	(*p) = malloc(sizeof(struct PtpGenericEvent) * length);

	uint8_t *dp = ptp_get_payload(r);
	for (int i = 0; i < length; i++) {
		// TODO: Simplify these triple pointers
		struct PtpGenericEvent *cur = &((*p)[i]);
		memset(cur, 0, sizeof(struct PtpGenericEvent));

		// Read header
		void *d = dp;
		uint32_t size = ptp_read_uint32((void **)&d);
		uint32_t type = ptp_read_uint32((void **)&d);

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
			uint32_t a = ptp_read_uint32(&d);
			uint32_t b = ptp_read_uint32(&d);
			cur->name = "request object transfer";
			cur->code = a;
			cur->value = b;
			} break;
		case PTP_EC_EOS_ObjectAddedEx: {
			struct PtpEOSObject *obj = (struct PtpEOSObject *)d;
			cur->name = "new object";
			cur->value = obj->a;
			} break;
		case PTP_EC_EOS_AvailListChanged: {
			uint32_t code = ptp_read_uint32(&d);
			uint32_t dat_type = ptp_read_uint32(&d);
			uint32_t count = ptp_read_uint32(&d);

			int payload_size = (size - 20);

			// Make sure to not divide by zero :)
			if (payload_size != 0 && count != 0) {
				int memb_size = payload_size / count;
			
				ptp_set_prop_avail_info(r, code, memb_size, count, d);
			}
			} break;
		}

		// Move dp over for the next entry
		dp += size;
	}

	return length;
}

int ptp_eos_events_json(struct PtpRuntime *r, char *buffer, int max) {
	struct PtpGenericEvent *events = NULL;

	int length = ptp_eos_events(r, &events);
	if (length == 0) {
		strncpy(buffer, "[]", max);
		return 2;
	}

	int curr = osnprintf(buffer, 0, max, "[");
	for (int i = 0; i < length; i++) {
		// Don't put comma for last entry
		char *end = ",";
		if (i - 1 == length) end = "";

		struct PtpGenericEvent *p = &(events[i]);
		
		if (p->name == NULL) {
			if (p->code == 0) continue;
			curr += osnprintf(buffer, curr, max, "[%u, %u]", p->code, p->value);
		} else {
			if (p->str_value == NULL) {
				curr += osnprintf(buffer, curr, max, "[\"%s\", %u]", p->name, p->value);
			} else {
				curr += osnprintf(buffer, curr, max, "[\"%s\", \"%s\"]", p->name, p->str_value);
			}
		}

		curr += osnprintf(buffer, curr, max, "%s\n", end);
	}

	curr += osnprintf(buffer, curr, max, "]");

	return curr;
}

int ptp_fuji_get_init_info(struct PtpRuntime *r, struct PtpFujiInitResp *resp) {
	void *dat = r->data + 12;

	resp->x1 = ptp_read_uint32(&dat);
	resp->x2 = ptp_read_uint32(&dat);
	resp->x3 = ptp_read_uint32(&dat);
	resp->x4 = ptp_read_uint32(&dat);

	ptp_read_unicode_string(resp->cam_name, (char *)(dat), sizeof(resp->cam_name));

	return 0;
}

int ptp_fuji_parse_object_info(struct PtpRuntime *r, struct PtpFujiObjectInfo *oi) {
	uint8_t *d = ptp_get_payload(r);
	memcpy(oi, d, PTP_FUJI_OBJ_INFO_VAR_START);
	d += PTP_FUJI_OBJ_INFO_VAR_START;
	ptp_read_string((void **)&d, oi->filename, sizeof(oi->filename));

	/* TODO: Figure out payload later:
		0D 44 00 53 00 43 00 46 00 35 00 30 00 38 00 37 00 2E 00 4A 00 50 00 47 00
		00 00 10 32 00 30 00 31 00 35 00 30 00 35 00 32 00 34 00 54 00 30 00 31 00
		31 00 37 00 31 00 30 00 00 00 00 0E 4F 00 72 00 69 00 65 00 6E 00 74 00 61
		00 74 00 69 00 6F 00 6E 00 3A 00 31 00 00 00 0C 00 00 00 03 00 01 20 0E 00
		00 00 00
	*/

	return 0;
}
