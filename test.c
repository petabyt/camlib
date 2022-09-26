#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>
#include <backend.h>
#include <ptp.h>
#include <operations.h>

struct PtpRuntime r;

void print_bytes(uint8_t *bytes, int n) {
	for (int i = 0; i < n; i++) {
		if (bytes[i] > 31 && bytes[i] < 128) {
			printf("'%c' ", bytes[i]);
		} else {
			printf("%02X ", bytes[i]);
		}
	}

	puts("");
}

void test();

int main() {
	r.data = malloc(4096);
	r.transaction = 0;
	r.session = 0;
	r.data_length = 4096;

	struct PtpDeviceInfo di;

	if (ptp_device_init()) {
		puts("Device init error");
		return 0;
	}

	//ptp_get_device_info(&r, &di);

	ptp_open_session(&r);

	print_bytes(r.data, 100);
	
	ptp_device_close();

	return 0;
}

