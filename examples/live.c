// Test EOS Liveview
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <libpict.h>

int main() {
	struct PtpRuntime r;
	ptp_init(&r);

	struct PtpDeviceInfo di;

	if (ptp_device_init(&r)) {
		puts("Device connection error");
		return 0;
	}

	int rc = ptp_open_session(&r);

	rc = ptp_get_device_info(&r, &di);

	ptp_eos_set_remote_mode(&r, 1);
	ptp_eos_set_event_mode(&r, 1);

	rc = ptp_liveview_init(&r);
	if (rc) {
		return 0;
	}

	void *lv = malloc(ptp_liveview_size(&r));

	while (1) {
		rc = ptp_liveview_frame(&r, lv);
		if (rc < 0) {
			printf("Error getting EOS frame: %d\n", rc);
			break;
		} else if (ptp_get_return_code(&r) == PTP_RC_OK) {
			ptp_dump(&r);
			printf("Got a valid frame");
			break;
		} else if (ptp_get_return_code(&r) == PTP_RC_CANON_NotReady) {
			printf("Not ready");
		}
	}

	free(lv);

	ptp_close_session(&r);
	ptp_device_close(&r);

	return 0;
}
