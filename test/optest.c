#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>
#include <backend.h>
#include <ptp.h>
#include <operations.h>

static struct PtpRuntime r;

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
	ptp_get_device_info(&r, &di);

	ptp_device_info_json(&di, (char*)r.data, r.data_length);
	printf("%s\n", (char*)r.data);

	struct PtpStorageIds sids;
	ptp_get_storage_ids(&r, &sids);
	printf("0x%X\n", sids.data[0]);

	struct PtpStorageInfo sinfo;
	if (ptp_get_storage_info(&r, &sinfo, sids.data[0])) {
		puts("ERR");
	}
	printf("%X\n", ptp_get_return_code(&r));
	printf("0x%lX\n", sinfo.free_space);

	ptp_close_session(&r);
	ptp_device_close(&r);

	free(r.data);

	return 0;
}
