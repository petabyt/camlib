// Bunch of handy util functions for CLI
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <camlib.h>

int ptp_list_devices(void) {
	struct PtpRuntime *r = ptp_new(PTP_USB);

	struct PtpDeviceEntry *list = ptpusb_device_list(r);

	if (list == NULL) {
		printf("No devices found\n");
		return 0;
	}

	for (; list != NULL; list = list->next) {
		printf("product id: %04x\n", list->product_id);
		printf("vendor id: %04x\n", list->vendor_id);
		printf("Vendor friendly name: '%s'\n", list->manufacturer);
		printf("Model friendly name: '%s'\n", list->name);
	}

	ptp_close(r);

	return 0;
}

static struct PtpRuntime *ptp_connect_id(int id) {
	struct PtpRuntime *r = ptp_new(PTP_USB);
	struct PtpDeviceEntry *list = ptpusb_device_list(r);

	if (list == NULL) {
		printf("No devices found\n");
		return 0;
	}

	int i = 0;
	for (; list != NULL; list = list->next) {
		if (i == id) {
			int rc = ptp_device_open(r, list);
			if (rc) return NULL;
			return r;
		}
		i++;
	}

	ptp_close(r);
	return NULL;
}

int ptp_dump_device(void) {
	struct PtpRuntime *r = ptp_connect_id(0);

	struct PtpDeviceInfo di;
	char buffer[4096];
	ptp_get_device_info(r, &di);
	ptp_device_info_json(&di, buffer, sizeof(buffer));
	printf("%s\n", (char*)buffer);

	return 0;
}
