// Parse/pack data and convert to/from JSON
#include <stdio.h>
#include <string.h>

#include <camlib.h>
#include <ptp.h>

int ptp_parse_object_info(struct PtpRuntime *r, struct PtpObjectInfo *oi) {
	void *d = ptp_get_payload(r);
	memcpy(d, oi, PTP_OBJ_INFO_VAR_START);
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

void *ptp_get_eos_event(struct PtpRuntime *r, void *d, struct PtpCanonEvent *ce) {
	void *d_ = d;

	uint32_t size = ptp_read_uint32(&d);
	ce->type = ptp_read_uint32(&d);

	if (ce->type == 0) return NULL;
	if (d >= (void*)ptp_get_payload(r) + ptp_get_data_length(r)) return NULL;

	int read = 0;
	switch (ce->type) {
	case PTP_EC_EOS_PropValueChanged:
		ce->code = ptp_read_uint32(&d);
		ce->value = ptp_read_uint32(&d);
		break;
	default:
		ce->code = 0;
		ce->value = 0;
		break;
	}

	// Return original unmodified by reads
	return d_ + size;
}
