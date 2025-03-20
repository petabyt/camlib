// Bunch of handy util functions for CLI
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <libpict.h>

int ptp_list_devices(void) {
	struct PtpRuntime *r = ptp_new(PTP_USB);

	struct PtpDeviceEntry *list = ptpusb_device_list(r);

	if (list == NULL) {
		printf("No devices found\n");
		return 0;
	}

	int i = 0;
	for (; list != NULL; list = list->next) {
		printf("Device #%d:\n", i);
		printf("  product id: %04x\n", list->product_id);
		printf("  vendor id: %04x\n", list->vendor_id);
		printf("  Vendor friendly name: '%s'\n", list->manufacturer);
		printf("  Model friendly name: '%s'\n", list->name);
		i++;
	}

	ptp_close(r);

	return 0;
}

struct PtpRuntime *ptp_connect_from_id(int id) {
	if (id == -1) {
		ptp_panic("%s Bug: -1", __func__);
	}
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

int ptp_dump_device(int dev_id) {
	struct PtpRuntime *r = ptp_connect_from_id(dev_id);

	struct PtpDeviceInfo di;
	char buffer[4096];
	ptp_get_device_info(r, &di);
	ptp_device_info_json(&di, buffer, sizeof(buffer));
	printf("%s\n", (char *)buffer);

	return 0;
}
