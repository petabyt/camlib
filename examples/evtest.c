// Test EOS events
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <libpict.h>

struct PtpRuntime r;

int main() {
	ptp_init(&r);

	if (ptp_device_init(&r)) {
		puts("Device connection error");
		return 0;
	}

	ptp_open_session(&r);

	struct PtpDeviceInfo di;

	ptp_get_device_info(&r, &di);
	char temp[4096];
	ptp_device_info_json(&di, temp, sizeof(temp));
	printf("%s\n", temp);

	int length = 0;
	struct PtpGenericEvent *s = NULL;
	if (ptp_device_type(&r) == PTP_DEV_EOS) {
		ptp_eos_set_remote_mode(&r, 1);
		ptp_eos_set_event_mode(&r, 1);

		int rc = ptp_eos_get_event(&r);
		if (rc) return rc;
		length = ptp_eos_events(&r, &s);

		for (int i = 0; i < length; i++) {
			if (s[i].code == 0) continue;
			printf("%X = %X\n", s[i].code, s[i].value);
		}
	}

	ptp_device_close(&r);
	return 0;
}

