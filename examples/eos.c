// Test dangerous Canon opcodes - only run this if you know what you're doing
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>

/* Some snippets, if you dare to run them:
Show green on display:
ptp_eos_evproc_run(&r, "UndisplayPhysicalScreen");
ptp_eos_evproc_run(&r, "CreateColor %d %d %d", 0, 70, 0);

Dump gangfile, all 0xFF:
ptp_eos_evproc_run(&r, "gang 'B:/ASD.BIN'");

Write to RAM (or ROM, if you're not careful):
ptp_eos_evproc_run(&r, "writeaddr %d %d", 0x3780, 0x12345678);
*/

int main() {
	struct PtpRuntime r;
	ptp_init(&r);

	struct PtpDeviceInfo di;

	if (ptp_device_init(&r)) {
		puts("Device connection error");
		return 0;
	}

	ptp_open_session(&r);

	ptp_eos_set_remote_mode(&r, 1);
	ptp_eos_set_event_mode(&r, 1);

	// Generate JSON data in packet buffer because I'm too lazy
	// to allocate a buffer (should be around 1-10kb of text)
	int rc = ptp_get_device_info(&r, &di);
	if (rc) return rc;
	r.di = &di;
	ptp_device_info_json(&di, (char*)r.data, r.data_length);
	printf("%s\n", (char*)r.data);

	// ...

	ptp_close_session(&r);
	ptp_device_close(&r);
	return 0;
}

