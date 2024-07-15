// Data structure unpacking and packing functions
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)
// This code uses unaligned access/writes, will probably be changed in the future

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <camlib.h>

// Custom snprint with offset
static int osnprintf(char *str, int cur, int size, const char *format, ...) {
	if (size - cur < 0) {
		ptp_panic("osnprintf overflow %d/%d", cur, size);
		return 0;
	}

	int r;
	va_list args;
	va_start(args, format);
	r = vsnprintf(str + cur, size - cur, format, args);
	va_end(args);

	return r;
}

int ptp_get_data_size(void *d, int type) {
	uint32_t length32;
	uint8_t length8;
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
		ptp_read_u32(d, &length32);
		return length32;
	case PTP_TC_STRING:
		ptp_read_u8(d, &length8);
		return length8;
	}

	ptp_panic("Invalid size read");
	return 0;
}

int ptp_parse_data_u32(void *d, int type, int *out) {
	uint8_t a;
	uint16_t b;
	uint32_t c;
	switch (type) {
	case PTP_TC_INT8:
	case PTP_TC_UINT8:
		ptp_read_u8(d, &a); (*out) = (int)a; return 1;
	case PTP_TC_INT16:
		ptp_read_u16(d, &b); (*out) = (int)((short)b); return 2;
	case PTP_TC_UINT16:
		ptp_read_u16(d, &b); (*out) = (int)b; return 2;
	case PTP_TC_INT32:
	case PTP_TC_UINT32:
		ptp_read_u32(d, &c); (*out) = (int)c; return 4;
	}

	// TODO: function to preserve signedness

	// skip the array
	return ptp_get_data_size(d, type);
}

int ptp_parse_prop_value(struct PtpRuntime *r) {
	int type;
	switch (ptp_get_payload_length(r)) {
	case 1:
		type = PTP_TC_UINT8; break;
	case 2:
		type = PTP_TC_UINT16; break;
	case 4:
		type = PTP_TC_UINT32; break;
	case 0:
		return -1;
	default:
		ptp_panic("ptp_parse_prop_value: unknown data type size");
	}

	int out;
	ptp_parse_data_u32(ptp_get_payload(r), type, &out);

	return out;
}

int parse_data_data_or_u32(uint8_t *d, int type, uint32_t *u32, void **data) {
	int size;
	uint32_t length32;
	uint8_t length8;
	int len_total;
	switch (type) {
	case PTP_TC_INT8:
	case PTP_TC_UINT8:
	case PTP_TC_INT16:
	case PTP_TC_UINT16:
	case PTP_TC_INT32:
	case PTP_TC_UINT32:
		return ptp_parse_data_u32(d, type, (int *)u32);
	case PTP_TC_INT64:
	case PTP_TC_UINT64:
		(*data) = malloc(8);
		memcpy((*data), d, 8);
		return 8;
	case PTP_TC_UINT8ARRAY:
		size = 1;
	case PTP_TC_UINT16ARRAY:
		size = 2;
	case PTP_TC_UINT32ARRAY:
		size = 4;
	case PTP_TC_UINT64ARRAY:
		size = 8;
		ptp_read_u32(d, &length32);
		(*data) = malloc(4 + length32 * size);
		memcpy((*data), d, 4 + length32 * size);
		return 4 + length32 * size;
	case PTP_TC_STRING:
		ptp_read_u8(d, &length8);
		len_total = 1 + (length8 * 2);
		(*data) = malloc(len_total);
		memcpy((*data), d, len_total);
		return len_total;
	default:
		ptp_panic("Unknown data type %d\n", type);
	}
}

int ptp_parse_prop_desc(struct PtpRuntime *r, struct PtpPropDesc *oi) {
	uint8_t *d = ptp_get_payload(r);

	d += ptp_read_u16(d, &oi->code);
	d += ptp_read_u16(d, &oi->data_type);
	d += ptp_read_u8(d, &oi->read_only);

	d += parse_data_data_or_u32(d, oi->data_type, &oi->default_value32, &oi->default_value);
	d += parse_data_data_or_u32(d, oi->data_type, &oi->current_value32, &oi->current_value);

	d += ptp_read_u8(d, &oi->form_type);

	if (oi->form_type == PTP_RangeForm) {
		d += ptp_parse_data_u32(d, oi->data_type, &oi->range_form.min);
		d += ptp_parse_data_u32(d, oi->data_type, &oi->range_form.max);
		d += ptp_parse_data_u32(d, oi->data_type, &oi->range_form.step);
	} else if (oi->form_type == PTP_EnumerationForm) {
		uint16_t num_values = 0;
		d += ptp_read_u16(d, &num_values);
		int length = 0;
		for (uint32_t i = 0; i < num_values; i++) {
			length += ptp_get_data_size(d + length, oi->data_type);
		}
		struct PtpEnumerationForm *form = (struct PtpEnumerationForm *)malloc(sizeof(struct PtpEnumerationForm) + length);
		form->length = num_values;
		memcpy(form->data, d, length);
		oi->enum_form = form;
	} else {
		ptp_panic("Unknown form type %d\n", oi->form_type);
		return -1;
	}

	return 0;
}

int ptp_prop_desc_json(struct PtpPropDesc *pd, char *buffer, int max) {
	int curr = osnprintf(buffer, curr, max, "{\n");
	curr += osnprintf(buffer, curr, max, "\"code\": %d,\n", pd->code);
	curr += osnprintf(buffer, curr, max, "\"type\": %d,\n", pd->data_type);

	switch (pd->data_type) {
		case PTP_TC_INT8:
		case PTP_TC_UINT8:
		case PTP_TC_INT16:
		case PTP_TC_UINT16:
		case PTP_TC_INT32:
		case PTP_TC_UINT32:
		case PTP_TC_INT64:
		case PTP_TC_UINT64:
			curr += osnprintf(buffer, curr, max, "\"currentValue32\": %d,\n", pd->current_value32);
			curr += osnprintf(buffer, curr, max, "\"defaultValue32\": %u,\n", pd->default_value32);
			break;
		case PTP_TC_UINT8ARRAY:
		case PTP_TC_UINT16ARRAY:
		case PTP_TC_UINT32ARRAY:
		case PTP_TC_UINT64ARRAY:
			curr += osnprintf(buffer, curr, max, "\"currentValueArray\": %d,\n", 0);
			break;
//		case PTP_TC_STRING:
	}

	if (pd->form_type == PTP_RangeForm) {
		curr += osnprintf(buffer, curr, max, "\"validEntries\": [");
		for (int i = pd->range_form.min; i < pd->range_form.max; i += pd->range_form.step) {
			char *end = ", ";
			if (i >= pd->range_form.max - pd->range_form.step) end = "";
			curr += osnprintf(buffer, curr, max, "%d%s", i, end);
		}
		curr += osnprintf(buffer, curr, max, "],\n");
	} else {
		curr += osnprintf(buffer, curr, max, "\"validEntries\": [");
		int of = 0;
		for (int i = 0; i < pd->enum_form->length; i++) {
			char *end = ", ";
			if (i >= pd->enum_form->length - 1) end = "";
			uint32_t value;
			void *data = NULL;
			of += parse_data_data_or_u32(pd->enum_form->data + of, pd->data_type, &value, &data);
			curr += osnprintf(buffer, curr, max, "%d%s", value, end);
		}
		curr += osnprintf(buffer, curr, max, "],\n");
	}

	curr += osnprintf(buffer, curr, max, "\"form_type\": %d\n", pd->form_type);

	curr += osnprintf(buffer, curr, max, "}");
	return curr;
}

int ptp_parse_object_info(struct PtpRuntime *r, struct PtpObjectInfo *oi) {
	uint8_t *d = ptp_get_payload(r);

	d += ptp_read_u32(d, &oi->storage_id);
	d += ptp_read_u16(d, &oi->obj_format);
	d += ptp_read_u16(d, &oi->protection);
	d += ptp_read_u32(d, &oi->compressed_size);
	d += ptp_read_u16(d, &oi->thumb_format);
	d += ptp_read_u32(d, &oi->thumb_compressed_size);
	d += ptp_read_u32(d, &oi->thumb_width);
	d += ptp_read_u32(d, &oi->thumb_height);
	d += ptp_read_u32(d, &oi->img_width);
	d += ptp_read_u32(d, &oi->img_height);
	d += ptp_read_u32(d, &oi->img_bit_depth);
	d += ptp_read_u32(d, &oi->parent_obj);
	d += ptp_read_u16(d, &oi->assoc_type);
	d += ptp_read_u32(d, &oi->assoc_desc);
	d += ptp_read_u32(d, &oi->sequence_num);

	d += ptp_read_string(d, oi->filename, sizeof(oi->filename));
	d += ptp_read_string(d, oi->date_created, sizeof(oi->date_created));
	d += ptp_read_string(d, oi->date_modified, sizeof(oi->date_modified));
	d += ptp_read_string(d, oi->keywords, sizeof(oi->keywords));

	return 0;
}

// TODO: Different API
int ptp_pack_object_info(struct PtpRuntime *r, struct PtpObjectInfo *oi, uint8_t *buf, int max) {
	if (1024 > max) {
		return 0;
	}

	int of = 0;
	of += ptp_write_u32(buf + of, oi->storage_id);
	of += ptp_write_u16(buf + of, oi->obj_format);
	of += ptp_write_u16(buf + of, oi->protection);
	of += ptp_write_u32(buf + of, oi->compressed_size);
	of += ptp_write_u16(buf + of, oi->thumb_format);
	of += ptp_write_u32(buf + of, oi->thumb_compressed_size);
	of += ptp_write_u32(buf + of, oi->thumb_width);
	of += ptp_write_u32(buf + of, oi->thumb_height);
	of += ptp_write_u32(buf + of, oi->img_width);
	of += ptp_write_u32(buf + of, oi->img_height);
	of += ptp_write_u32(buf + of, oi->img_bit_depth);
	of += ptp_write_u32(buf + of, oi->parent_obj);
	of += ptp_write_u16(buf + of, oi->assoc_type);
	of += ptp_write_u32(buf + of, oi->assoc_desc);
	of += ptp_write_u32(buf + of, oi->sequence_num);

	// If the string is empty, don't add it to the packet
	if (oi->filename[0] != '\0')
		of += ptp_write_string(buf + of, oi->filename);
	if (oi->date_created[0] != '\0')
		of += ptp_write_string(buf + of, oi->date_created);
	if (oi->date_modified[0] != '\0')
		of += ptp_write_string(buf + of, oi->date_modified);
	if (oi->keywords[0] != '\0')
		of += ptp_write_string(buf + of, oi->keywords);

	// Return pointer length added
	return of;
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
	uint8_t *data = malloc(size_all);
	if (data == NULL) return NULL;

	int of = 0;
	of += ptp_write_u32(data + of, strlen(out) + 1);
	of += ptp_write_utf8_string(data + of, out);

	fread(data + of, 1, file_size, f);

	return data;
}

int ptp_parse_device_info(struct PtpRuntime *r, struct PtpDeviceInfo *di) {
	uint8_t *b = ptp_get_payload(r);
	uint8_t *e = b + r->data_length;

	b += ptp_read_u16(b, &di->standard_version);
	b += ptp_read_u32(b, &di->vendor_ext_id);
	b += ptp_read_u16(b, &di->version);

	b += ptp_read_string(b, di->extensions, sizeof(di->extensions));
	b += ptp_read_u16(b, &di->functional_mode);

	b += ptp_read_uint16_array_s(b, e, di->ops_supported, sizeof(di->ops_supported) / 2, &di->ops_supported_length);
	b += ptp_read_uint16_array_s(b, e, di->events_supported, sizeof(di->events_supported) / 2, &di->events_supported_length);
	b += ptp_read_uint16_array_s(b, e, di->props_supported, sizeof(di->props_supported) / 2, &di->props_supported_length);
	b += ptp_read_uint16_array_s(b, e, di->capture_formats, sizeof(di->capture_formats) / 2, &di->capture_formats_length);
	b += ptp_read_uint16_array_s(b, e, di->playback_formats, sizeof(di->playback_formats) / 2, &di->playback_formats_length);

	b += ptp_read_string(b, di->manufacturer, sizeof(di->manufacturer));	
	b += ptp_read_string(b, di->model, sizeof(di->model));

	b += ptp_read_string(b, di->device_version, sizeof(di->device_version));
	b += ptp_read_string(b, di->serial_number, sizeof(di->serial_number));

	r->di = di; // set last parsed di

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

static const char *eval_obj_format(int code) {
	char *x = ptp_get_enum_all(code);
	switch (code) {
	case PTP_OF_Association:
		return "folder";
	default:
		if (x) return x;
		return "none";
	}
}

static const char *eval_protection(int code) {
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
	curr += osnprintf(buffer, curr, max, "\"storage_id\": %u,", so->storage_id);
	curr += osnprintf(buffer, curr, max, "\"compressedSize\": %u,", so->compressed_size);
	curr += osnprintf(buffer, curr, max, "\"parent\": %u,", so->parent_obj);
	curr += osnprintf(buffer, curr, max, "\"format\": \"%s\",", eval_obj_format(so->obj_format));
	curr += osnprintf(buffer, curr, max, "\"format_int\": %u,", so->obj_format);
	curr += osnprintf(buffer, curr, max, "\"protection\": \"%s\",", eval_protection(so->protection));
	curr += osnprintf(buffer, curr, max, "\"filename\": \"%s\",", so->filename);
	if (so->compressed_size != 0) {
		curr += osnprintf(buffer, curr, max, "\"imgWidth\": %u,", so->img_width);
		curr += osnprintf(buffer, curr, max, "\"imgHeight\": %u,", so->img_height);
	}
	curr += osnprintf(buffer, curr, max, "\"dateCreated\": \"%s\",", so->date_created);
	curr += osnprintf(buffer, curr, max, "\"dateModified\": \"%s\"", so->date_modified);
	curr += osnprintf(buffer, curr, max, "}");

	return curr;
}

static const char *eval_storage_type(int id) {
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

int ptp_eos_prop_next(uint8_t *d, struct PtpGenericEvent *p) {
	uint32_t code, value, tmp;

	int of = 0;
	of += ptp_read_u32(d + of, &code);
	of += ptp_read_u32(d + of, &value);

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
			uint32_t data[5] = {value};
			of += ptp_read_u32(d + of, &data[1]);
			of += ptp_read_u32(d + of, &data[2]);
			of += ptp_read_u32(d + of, &data[3]);
			of += ptp_read_u32(d + of, &data[4]);
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
		uint8_t *d = dp;
		uint32_t size, type;
		d += ptp_read_u32(d, &size);
		d += ptp_read_u32(d, &type);

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
	struct PtpGenericEvent *p_base = (*p);

	uint8_t *dp = ptp_get_payload(r);
	for (int i = 0; i < length; i++) {
		struct PtpGenericEvent *cur = &p_base[i];
		memset(cur, 0, sizeof(struct PtpGenericEvent));

		uint8_t *d = dp;

		uint32_t size, type;
		d += ptp_read_u32(d, &size);
		d += ptp_read_u32(d, &type);

		// Detect termination or overflow
		if (type == 0) break;

		switch (type) {
		case PTP_EC_EOS_PropValueChanged:
			d += ptp_eos_prop_next(d, cur);
			break;
		case PTP_EC_EOS_InfoCheckComplete:
		case PTP_PC_EOS_FocusInfoEx:
			cur->name = ptp_get_enum_all(type);
			break;
		case PTP_EC_EOS_RequestObjectTransfer: {
			uint32_t a, b;
			d += ptp_read_u32(d, &a);
			d += ptp_read_u32(d, &b);
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
			uint32_t code, dat_type, count;
			d += ptp_read_u32(d, &code);
			d += ptp_read_u32(d, &dat_type);
			d += ptp_read_u32(d, &count);

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
