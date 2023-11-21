// Scan filesystem
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

	char buffer[4096];
	ptp_get_device_info(&r, &di);
	ptp_device_info_json(&di, buffer, sizeof(buffer));
	printf("%s\n", (char*)buffer);

	ptp_close_session(&r);
	ptp_device_close(&r);
	ptp_generic_close(&r);
	return 0;
}

