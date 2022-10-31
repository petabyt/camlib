// Parses device info directly from packet
#ifndef PTP_DEVICE_INFO_H
#define PTP_DEVICE_INFO_H

#include "camlib.h"
#include <stdio.h>

int ptp_parse_device_info(struct PtpRuntime *r, struct PtpDeviceInfo *di) {
	// Skip packet header
	void *e = (r->data + 12);

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

	// TODO: manufacturer, model, device_version, serial_number strings

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

#endif
