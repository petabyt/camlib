// Test basic opcode, get device properties
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>

int ptp_set_prop_value_16(struct PtpRuntime *r, int code, int value) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_SetDevicePropValue;
	cmd.param_length = 1;
	cmd.params[0] = code;

	uint16_t dat[] = {(uint16_t)value};

	return ptp_generic_send_data(r, &cmd, dat, sizeof(dat));
}

int main() {
	struct PtpRuntime r;
	ptp_generic_init(&r);
	r.connection_type = PTP_IP;
	if (ptpip_connect(&r, "192.168.1.2", PTP_IP_PORT)) {
		puts("Device connection error");
		return 0;
	}

	if (ptpip_init_command_request(&r, "camlib")) {
		puts("Error on initialize");
		return 1;
	}

	puts("Done initing");

	if (ptpip_connect_events(&r, "192.168.1.2", PTP_IP_PORT)) {
		puts("Events connection error");
		return 0;
	}

	printf("Waiting to send event ask\n");

	if (ptpip_init_events(&r)) {
		printf("Event init error\n");
		return 1;
	}

	ptp_open_session(&r);

	ptp_eos_set_remote_mode(&r, 1);
	ptp_eos_set_event_mode(&r, 1);

	struct PtpDeviceInfo di;

	char buffer[4096];
	int rc = ptp_get_device_info(&r, &di);
	if (rc) {
		puts("Failed to get device info\n");
		return rc;
	}

	ptp_device_info_json(&di, buffer, sizeof(buffer));
	printf("%s\n", buffer);

	ptp_close_session(&r);

	ptpip_close(&r);
	return 0;
}

