// Test Magic Lantern things
#include <stdio.h>
#include <libpict.h>

int main(int argc, char **argv) {
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

	ptp_chdk_get_version(&r);
	// Upload the file with varargs
	ptp_chdk_upload_file(&r, argv[1], argv[2]);

	ptp_close_session(&r);
	ptp_device_close(&r);
	return 0;
}

