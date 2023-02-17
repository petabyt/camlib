#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>

#define SIZE 300000
int main() {
	struct PtpRuntime r;

	memset(&r, 0, sizeof(struct PtpRuntime));
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

	// ptp_eos_remote_release_on(&r, 1);
	// ptp_eos_remote_release_on(&r, 2);
	// ptp_eos_remote_release_off(&r, 2);
	// ptp_eos_remote_release_off(&r, 1);

	ptp_close_session(&r);
	ptp_device_close(&r);

	free(r.data);

	return 0;
}

