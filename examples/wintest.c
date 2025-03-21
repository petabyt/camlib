// libwpd test
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <libpict.h>
#include <ptp.h>

int main() {
	struct PtpRuntime r;
	ptp_init(&r);

	struct PtpDeviceInfo di;

	if (ptp_device_init(&r)) {
		printf("Couldn't find a device.\n");
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
		printf("%s\n", (char *)r.data);
	}

	ptp_eos_set_event_mode(&r, 1);
	ptp_eos_set_remote_mode(&r, 1);

	int ret = ptp_eos_set_prop_value(&r, PTP_DPC_EOS_ISOSpeed, ptp_eos_get_iso(100, 1));
	if (ret) {
		printf("error setting prop %d\n", ret);
	}

	ptp_device_close(&r);

	printf("Press any key to close\n");
	getchar();

	return 0;
}
