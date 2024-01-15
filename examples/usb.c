// Test device list API
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>

static int connect(struct PtpRuntime *r) {
	struct PtpDeviceEntry *list = ptpusb_device_list(r);

	printf("Starting list\n");
	for (struct PtpDeviceEntry *curr = list; curr != NULL; curr = curr->next) {
		printf("Device: %s\tVendor: \t%X\n", curr->name, curr->vendor_id);
	}

	if (ptp_device_init(r)) {
		puts("Device connection error");
		return 1;
	}

	ptp_open_session(r);
	ptp_close_session(r);

	ptp_device_close(r);
	return 0;
}

int main() {
	struct PtpRuntime r;
	ptp_generic_init(&r);
	ptp_comm_init(&r);

	connect(&r);
	connect(&r);

	return 0;
}

