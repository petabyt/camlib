#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>
#include <backend.h>
#include <ptp.h>
#include <operations.h>
#include <winapi.h>

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

	ptp_generic_init(&r);
	r.data = malloc(SIZE);
	r.data_length = SIZE;

	struct PtpDeviceInfo di;

	if (ptp_device_init(&r)) {
		puts("Couldn't find a device.");
		return 0;
	}

	if (ptp_open_session(&r)) {
		printf("IO error\n");
		return 1;
	}

	printf("Opened session, rcode: %X\n", ptp_get_return_code(&r));

	if (ptp_get_device_info(&r, &di)) {
		puts("IO ERR");
	} else {
		printf("Return Code: %X\n", ptp_get_return_code(&r));
		ptp_device_info_json(&di, (char*)r.data, r.data_length);
		printf("%s\n", (char*)r.data);
	}

	ptp_eos_set_event_mode(&r, 1);
	ptp_eos_set_remote_mode(&r, 1);

	int ret = ptp_eos_set_prop_value(&r, PTP_PC_EOS_ISOSpeed, ptp_eos_get_iso(100, 1));
	if (ret) {
		printf("error setting prop %d\n", ret);
	}

	ptp_device_close(&r);

	free(r.data);

	getchar();

	return 0;
}
