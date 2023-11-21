// Test device list API
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>

int main() {
	struct PtpRuntime r;
	ptp_comm_init(&r);

	struct PtpDeviceEntry *list = ptpusb_device_list(&r);

	printf("Starting list\n");
	for (struct PtpDeviceEntry *curr = list; curr != NULL; curr = curr->next) {
		printf("Device: %s\tVendor: \t%X\n", curr->name, curr->vendor_id);
	}

	if (ptp_device_init(&r)) {
		puts("Device connection error");
		return 0;
	}

	ptp_open_session(&r);
	ptp_close_session(&r);

	ptp_device_close(&r);
	return 0;
}

