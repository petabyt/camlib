// Test basic opcode, get device properties
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>

int main() {
	struct PtpRuntime r;
	ptp_generic_init(&r);
	r.connection_type = PTP_IP;
	if (ptpip_connect(&r, "192.168.0.1", FUJI_CMD_IP_PORT)) {
		puts("Device connection error");
		return 0;
	}

	if (ptpip_fuji_init(&r, "camlib")) {
		puts("Error on initialize");
	}

	// if (ptpip_connect_events(&r, "192.168.0.1", FUJI_CMD_IP_PORT)) {
		// puts("Device event connection error");
		// return 0;
	// }
// 
	// if (ptpip_init_events(&r)) {
		// puts("Error on events initialize");
		// return 0;
	// }

	struct PtpFujiInitResp resp;
	ptp_fuji_get_init_info(&r, &resp);

	printf("Connecting to %s\n", resp.cam_name);

	ptp_open_session(&r);

	ptpip_fuji_wait_unlocked(&r);

	ptp_set_prop_value(&r, PTP_PC_Fuji_Mode, 2);
	ptp_set_prop_value(&r, PTP_PC_Fuji_TransferMode, 2);

	for (int i = 0; i < 100; i++) {
		puts("Pinging");
		if (ptp_fuji_ping(&r)) break;
		usleep(1000 * 1000);
	}

	// Camera shuts down after disconnect, would be nice to prevent that

	ptpip_close(&r);

	free(r.data);
	return 0;
}

