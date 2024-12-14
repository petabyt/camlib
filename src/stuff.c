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

	return 0;
}