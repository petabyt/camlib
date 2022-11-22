#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>
#include <backend.h>
#include <ptp.h>
#include <operations.h>

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

#define SIZE 3000000

int main() {
	struct PtpRuntime r;

	memset(&r, sizeof(struct PtpRuntime), 0);
	r.data = malloc(SIZE);
	r.transaction = 0;
	r.session = 0;
	r.data_length = SIZE;

	struct PtpDeviceInfo di;

	if (ptp_device_init(&r)) {
		puts("Device connection error");
		return 0;
	}

	ptp_open_session(&r);



	ptp_close_session(&r);
	ptp_device_close(&r);

	free(r.data);

	return 0;
}

