// Test packet creation
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>

void print_bytes(uint8_t *bytes, int n) {
	for (int i = 0; i < n; i++) {
		printf("%02X ", bytes[i]);
		continue;
		if (bytes[i] > 31 && bytes[i] < 128) {
			printf("'%c' ", bytes[i]);
		} else {
			printf("%02X ", bytes[i]);
		}
	}

	puts("");
}

int main() {
	struct PtpRuntime r;
	ptp_init(&r);

	r.connection_type = PTP_IP;

	struct PtpCommand cmd;
	cmd.code = PTP_OC_OpenSession;
	cmd.params[0] = 1;
	cmd.param_length = 1;

	int l = ptp_new_cmd_packet(&r, &cmd);
	print_bytes(r.data, l);

	return 0;
}

