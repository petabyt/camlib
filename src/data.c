// Parse/pack data and convert to (and from?) JSON
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)

#include <stdio.h>
#include <string.h>

#include <camlib.h>
#include <ptp.h>
#include <enum.h>

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

int ptp_pack_object_info(struct PtpRuntime *r, struct PtpObjectInfo *oi) {
	void *d = ptp_get_payload(r);
	memcpy(oi, d, PTP_OBJ_INFO_VAR_START);
	d += PTP_OBJ_INFO_VAR_START;

	ptp_write_string(&d, oi->filename);
	ptp_write_string(&d, oi->date_created);
	ptp_write_string(&d, oi->date_modified);
	ptp_write_string(&d, oi->keywords);

	return 0;
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
	curr += snprintf(buffer + curr, max - curr, "    \"model\": \"%s\",\n", di->model);
	curr += snprintf(buffer + curr, max - curr, "    \"device_version\": \"%s\",\n", di->device_version);
	curr += snprintf(buffer + curr, max - curr, "    \"serial_number\": \"%s\"\n", di->serial_number);
	curr += snprintf(buffer + curr, max - curr, "}");
	return curr;
}

const static char *eval_obj_format(int code) {
	char *x = ptp_get_enum(code);
	switch (code) {
	case PTP_OF_Association:
		return "folder";
	default:
		if (x) return x;
		return "none";
	}
}

const static char *eval_protection(int code) {
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
	curr += sprintf(buffer + curr, "\"storage_id\": %u,", so->storage_id);
	curr += sprintf(buffer + curr, "\"parent\": %u,", so->parent_obj);
	curr += sprintf(buffer + curr, "\"format\": \"%s\",", eval_obj_format(so->obj_format));
	curr += sprintf(buffer + curr, "\"format_int\": %u,", so->obj_format);
	curr += sprintf(buffer + curr, "\"protection\": \"%s\",", eval_protection(so->protection));
	curr += sprintf(buffer + curr, "\"filename\": \"%s\",", so->filename);
	if (so->compressed_size != 0) {
		curr += sprintf(buffer + curr, "\"img_width\": %u,", so->img_width);
		curr += sprintf(buffer + curr, "\"img_height\": %u,", so->img_height);
	}
	curr += sprintf(buffer + curr, "\"date_created\": \"%s\",", so->date_created);
	curr += sprintf(buffer + curr, "\"date_modified\": \"%s\"", so->date_modified);
	curr += sprintf(buffer + curr, "}");

	return curr;
}

//int ptp_parse_object_info(struct PtpRuntime *r, struct PtpObjectInfo *oi) {

/*
int x = ptp_open_eos_events(r);
if (x == NULL) return;
x = ptp_get_eos_event(r, &event);
if (x == NULL) goto end;
*/

// Returns positive offset, or NULL
void *ptp_open_eos_events(struct PtpRuntime *r) {
	return ptp_get_payload(r);
}

// Returns NUL when on last element
void *ptp_get_eos_event(struct PtpRuntime *r, void *d, struct PtpCanonEvent *ce) {
	void *d_ = d;
	ce->code = 0;
	ce->value = 0;

	uint32_t size = ptp_read_uint32(&d);
	ce->type = ptp_read_uint32(&d);

	if (ce->type == 0) return NULL;
	if (d >= (void*)ptp_get_payload(r) + ptp_get_data_length(r)) return NULL;

	switch (ce->type) {
	case PTP_EC_EOS_PropValueChanged:
		ce->code = ptp_read_uint32(&d);
		ce->value = ptp_read_uint32(&d);
		break;
	}

	// Return original unmodified by reads
	return d_ + size;
}

int ptp_eos_events_json(struct PtpRuntime *r, char *buffer, int max) {
	struct PtpCanonEvent ce;
	void *dp = ptp_open_eos_events(r);

	int curr = sprintf(buffer, "[\n");

	int tmp = 0;
	while (dp != NULL) {
		dp = ptp_get_eos_event(r, dp, &ce);

		if (ce.code == 0) continue;

		// Don't put comma for last entry
		char *end = "";
		if (tmp) end = ",";
		tmp = 1;

		if (dp == NULL) break;

		char *name = ptp_get_enum(ce.code);
		char *value = NULL;
		switch (ce.code) {
		case PTP_PC_EOS_Aperture:
			ce.value = ptp_eos_get_aperture(ce.value, 0);
			name = "aperture";
			break;
		case PTP_PC_EOS_ShutterSpeed:
			ce.value = ptp_eos_get_shutter(ce.value, 0);
			name = "shutter speed";
			break;
		case PTP_PC_EOS_ISOSpeed:
			ce.value = ptp_eos_get_iso(ce.value, 0);
			name = "iso";
			break;
		case PTP_PC_EOS_BatteryPower:
			name = "battery";
			break;
		case PTP_PC_EOS_ImageFormat:
			ce.value = ptp_eos_get_imgformat(ce.value, 0);
			name = "image format";
		}

		if (name == enum_null) {
			curr += snprintf(buffer + curr, max - curr, "    %s[%u, %u]\n", end, ce.code, ce.value);
		} else {
			if (value == NULL) {
				curr += snprintf(buffer + curr, max - curr, "    %s[\"%s\", %u]\n", end, name, ce.value);
			} else {
				curr += snprintf(buffer + curr, max - curr, "    %s[\"%s\", \"%s\"]\n", end, name, value);
			}
		}

		if (curr >= max) return 0;
	}

	curr += sprintf(buffer + curr, "]");
	return curr;
}

struct CanonShutterSpeed {
	int value; // value times 100000
	int data; // data from camera
}canon_shutter[] = {
	{0, 0xc}, // BULB 1300D
	{0, 0x4}, // BULB 5dmk3
	{3000000,0x10},
	{2500000,0x13},
	{2000000,0x15},
	{1500000,0x18},
	{1300000,0x1b},
	{1000000,0x1d},
	{800000,0x20},
	{600000,0x23},
	{500000,0x25},
	{400000,0x28},
	{320000,0x2b},
	{250000,0x2d},
	{200000,0x30},
	{160000,0x33},
	{130000,0x35},
	{100000,0x38},
	{80000,0x3b},
	{60000,0x3d},
	{50000,0x40},
	{40000,0x43},
	{30000,0x45},
	{25000,0x48},
	{20000,0x4b},
	{16666,0x4d},
	{12500,0x50},
	{100000 / 10,0x53},
	{100000 / 13,0x55},
	{100000 / 15,0x58},
	{100000 / 20,0x5b},
	{100000 / 25,0x5d},
	{100000 / 30,0x60},
	{100000 / 40,0x63},
	{100000 / 50,0x65},
	{100000 / 60,0x68},
	{100000 / 80,0x6b},
	{100000 / 100,0x6d},
	{100000 / 125,0x70},
	{100000 / 160,0x73},
	{100000 / 200,0x75},
	{100000 / 250,0x78},
	{100000 / 320,0x7b},
	{100000 / 400,0x7d},
	{100000 / 500,0x80},
	{100000 / 640,0x83},
	{100000 / 800,0x85},
	{100000 / 1000,0x88},
	{100000 / 1250,0x8b},
	{100000 / 1600,0x8d},
	{100000 / 2000,0x90},
	{100000 / 2500,0x93},
	{100000 / 3200,0x95},
	{100000 / 4000,0x98},
	{100000 / 5000,0x9a},
	{100000 / 6400,0x9d},
	{100000 / 8000,0xa0},
};

int ptp_eos_get_shutter(int data, int dir) {
	for (int i = 0; i < sizeof(canon_shutter) / sizeof(struct CanonShutterSpeed); i++) {
		if (dir) {
			if (canon_shutter[i].value == data) {
				return canon_shutter[i].data;
			}
		} else {
			if (canon_shutter[i].data == data) {
				return canon_shutter[i].value;
			}
		}
	}

	return data;
}

struct CanonISO {
	int value;
	int data;
}canon_iso[] = {
	{0, 0}, // AUTO
	{50, 0x40},
	{100, 0x48},
	{125, 0x4b},
	{160, 0x4d},
	{200, 0x50},
	{250, 0x53},
	{320, 0x55},
	{400, 0x58},
	{500, 0x5b},
	{640, 0x5b},
	{800, 0x60},
	{1000, 0x63},
	{1250, 0x65},
	{1600, 0x68},
	{3200, 0x70},
	{6400, 0x78},
	{12800, 0x78+8},
};

int ptp_eos_get_iso(int data, int dir) {
	for (int i = 0; i < sizeof(canon_iso) / sizeof(struct CanonISO); i++) {
		if (dir) {
			if (canon_iso[i].value == data) {
				return canon_iso[i].data;
			}
		} else {
			if (canon_iso[i].data == data) {
				return canon_iso[i].value;
			}
		}
	}

	return data;
}

struct CanonAperture {
	int value;
	int data;
}canon_aperture[] = {
	{12, 0xd},
	{14, 0x10},
	{16, 0x13},
	{18, 0x15},
	{20, 0x18},
	{22, 0x1b},
	{25, 0x1d},
	{28, 0x20},
	{32, 0x23},
	{35, 0x25},
	{40, 0x28},
	{45, 0x2b},
	{50, 0x2d},
	{56, 0x30},
	{71, 0x33},
	{80, 0x35},
	{90, 0x38},
	{100, 0x3b},
	{110, 0x40},
	{130, 0x43},
	{140, 0x45},
	{160, 0x48},
	{180, 0x4b},
	{200, 0x4d},
	{220, 0x50},
	{250, 0x53},
	{290, 0x55},
	{320, 0x58},
};

int ptp_eos_get_aperture(int data, int dir) {
	for (int i = 0; i < sizeof(canon_aperture) / sizeof(struct CanonAperture); i++) {
		if (dir) {
			if (canon_aperture[i].value == data) {
				return canon_aperture[i].data;
			}
		} else {
			if (canon_aperture[i].data == data) {
				return canon_aperture[i].value;
			}
		}
	}

	return data;
}

// Lots of confusing types (resolutions, raw+jpeg, superfine, etc)
// See enum EOSImageFormats
struct CanonImageFormats {
	int value;
	int data;
}canon_imgformats[] = {
	{1, 0}, // RAW
	{3, 1}, // RAW/JPG
	// 2
	// 3
	// 4
	// 5
	// 6
	// 7
	{2, 8}, // JPG
	// 9
	// 10
	// 11
	// 12
	// 13
};

int ptp_eos_get_imgformat(int data, int dir) {
	for (int i = 0; i < sizeof(canon_imgformats) / sizeof(struct CanonImageFormats); i++) {
		if (dir) {
			if (canon_imgformats[i].value == data) {
				return canon_imgformats[i].data;
			}
		} else {
			if (canon_imgformats[i].data == data) {
				return canon_imgformats[i].value;
			}
		}
	}

	return 0;
}
