// Scan filesystem using JSON bindings
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>

int main() {
	struct PtpRuntime r;
	ptp_generic_init(&r);

	char buffer[PTP_BIND_DEFAULT_SIZE];
	int rc = bind_run(&r, "ptp_init", buffer, sizeof(buffer));
	printf("ptp_init: %s\n", buffer);

	rc = bind_run(&r, "ptp_connect", buffer, sizeof(buffer));
	printf("ptp_connect: %s\n", buffer);

	rc = bind_run(&r, "ptp_open_session", buffer, sizeof(buffer));
	printf("ptp_open_session: %s\n", buffer);

	rc = bind_run(&r, "ptp_get_device_info", buffer, sizeof(buffer));
	printf("ptp_get_device_info: %s\n", buffer);

	rc = bind_run(&r, "ptp_close_session", buffer, sizeof(buffer));
	printf("ptp_close_session: %s\n", buffer);

	rc = bind_run(&r, "ptp_disconnect", buffer, sizeof(buffer));
	printf("ptp_disconnect: %s\n", buffer);

	ptp_generic_close(&r);
	return 0;
}
