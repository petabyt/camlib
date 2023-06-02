// Test basic opcode, get device properties
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>
#include <fuji.h>

int main() {
	struct PtpRuntime r;
	ptp_generic_init(&r);
	r.connection_type = PTP_IP;

	if (ptpip_connect(&r, "192.168.0.1", 55740)) {
		puts("Device connection error");
		return 0;
	}

	ptp_fuji_init(&r, "camlib");

	ptp_open_session(&r);

	ptp_fuji_wait_unlocked(&r);

	ptp_set_prop_value(&r, PTP_PC_Fuji_Mode, 2);
	ptp_set_prop_value(&r, PTP_PC_Fuji_TransferMode, 2);

	for (int i = 0; i < 100; i++) {
		puts("Pinging");
		if (ptpip_ping(&r)) break;
		usleep(1000 * 1000);
	}

	//ptpip_close(&r);

	free(r.data);
	return 0;
}

