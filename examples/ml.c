// Test Magic Lantern things
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>

int main() {
	struct PtpRuntime r;
	ptp_generic_init(&r);

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

	ptp_chdk_get_version(&r);
	ptp_chdk_upload_file(&r, "/home/daniel/Documents/ptpview/ptpview.mo", "B:/ML/MODULES/PTPVIEW.MO");

	ptp_close_session(&r);
	ptp_device_close(&r);
	return 0;
}

