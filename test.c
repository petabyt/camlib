#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <piclib.h>
#include <ptp.h>
#include <deviceinfo.h>

struct PtpRuntime r;

// 512 byte response from GetDeviceInfo on Canon 1300D
uint8_t export[] = {0xb, 0x2, 0x0, 0x0, 0x2, 0x0, 0x1, 0x10, 0x1, 0x0, 0x0, 0x0, 0x64, 0x0, 0x6, 0x0, 0x0, 0x0, 0x64, 0x0, 0x0, 0x0, 0x0, 0xa7, 0x0, 0x0, 0x0, 0x14, 0x10, 0x15, 0x10, 0x16, 0x10, 0x1, 0x10, 0x2, 0x10, 0x3, 0x10, 0x6, 0x10, 0x4, 0x10, 0x1, 0x91, 0x5, 0x10, 0x2, 0x91, 0x7, 0x10, 0x8, 0x10, 0x3, 0x91, 0x9, 0x10, 0x4, 0x91, 0xa, 0x10, 0x1b, 0x10, 0x7, 0x91, 0xc, 0x10, 0xd, 0x10, 0xb, 0x10, 0x5, 0x91, 0xf, 0x10, 0x6, 0x91, 0x10, 0x91, 0x27, 0x91, 0xb, 0x91, 0x8, 0x91, 0x9, 0x91, 0xc, 0x91, 0xe, 0x91, 0xf, 0x91, 0x15, 0x91, 0x14, 0x91, 0x13, 0x91, 0x16, 0x91, 0x17, 0x91, 0x20, 0x91, 0xf0, 0x91, 0x18, 0x91, 0x21, 0x91, 0xf1, 0x91, 0x1d, 0x91, 0xa, 0x91, 0x1b, 0x91, 0x1c, 0x91, 0x1e, 0x91, 0x1a, 0x91, 0x53, 0x91, 0x54, 0x91, 0x60, 0x91, 0x55, 0x91, 0x57, 0x91, 0x58, 0x91, 0x59, 0x91, 0x5a, 0x91, 0x1f, 0x91, 0xfe, 0x91, 0xff, 0x91, 0x28, 0x91, 0x29, 0x91, 0x2d, 0x91, 0x2e, 0x91, 0x2f, 0x91, 0x2c, 0x91, 0x30, 0x91, 0x31, 0x91, 0x32, 0x91, 0x33, 0x91, 0x34, 0x91, 0x2b, 0x91, 0x35, 0x91, 0x36, 0x91, 0x37, 0x91, 0x38, 0x91, 0x39, 0x91, 0x3a, 0x91, 0x3b, 0x91, 0x3c, 0x91, 0xda, 0x91, 0xdb, 0x91, 0xdc, 0x91, 0xdd, 0x91, 0xde, 0x91, 0xd8, 0x91, 0xd9, 0x91, 0xd7, 0x91, 0xd5, 0x91, 0x2f, 0x90, 0x41, 0x91, 0x42, 0x91, 0x43, 0x91, 0x3f, 0x91, 0x33, 0x90, 0x68, 0x90, 0x69, 0x90, 0x6a, 0x90, 0x6b, 0x90, 0x6c, 0x90, 0x6d, 0x90, 0x6e, 0x90, 0x6f, 0x90, 0x3d, 0x91, 0x80, 0x91, 0x81, 0x91, 0x82, 0x91, 0x83, 0x91, 0x84, 0x91, 0x85, 0x91, 0x40, 0x91, 0x1, 0x98, 0x2, 0x98, 0x3, 0x98, 0x4, 0x98, 0x5, 0x98, 0xc0, 0x91, 0xc1, 0x91, 0xc2, 0x91, 0xc3, 0x91, 0xc4, 0x91, 0xc5, 0x91, 0xc6, 0x91, 0xc7, 0x91, 0xc8, 0x91, 0xc9, 0x91, 0xca, 0x91, 0xcb, 0x91, 0xcc, 0x91, 0xce, 0x91, 0xcf, 0x91, 0xd0, 0x91, 0xd1, 0x91, 0xd2, 0x91, 0xe1, 0x91, 0xe2, 0x91, 0xe3, 0x91, 0xe4, 0x91, 0xe5, 0x91, 0xe6, 0x91, 0xe7, 0x91, 0xe8, 0x91, 0xe9, 0x91, 0xea, 0x91, 0xeb, 0x91, 0xec, 0x91, 0xed, 0x91, 0xee, 0x91, 0xef, 0x91, 0xf8, 0x91, 0xf9, 0x91, 0xf2, 0x91, 0xf3, 0x91, 0xf4, 0x91, 0xf7, 0x91, 0x22, 0x91, 0x23, 0x91, 0x24, 0x91, 0xf5, 0x91, 0xf6, 0x91, 0x52, 0x90, 0x53, 0x90, 0x57, 0x90, 0x58, 0x90, 0x59, 0x90, 0x5a, 0x90, 0x5f, 0x90, 0x7, 0x0, 0x0, 0x0, 0x9, 0x40, 0x4, 0x40, 0x5, 0x40, 0x3, 0x40, 0x2, 0x40, 0x7, 0x40, 0x1, 0xc1, 0x5, 0x0, 0x0, 0x0, 0x2, 0xd4, 0x7, 0xd4, 0x6, 0xd4, 0x3, 0xd3, 0x1, 0x50, 0x1, 0x0, 0x0, 0x0, 0x1, 0x38, 0xc, 0x0, 0x0, 0x0, 0x1, 0x30, 0x2, 0x30, 0x6, 0x30, 0xa, 0x30, 0x8, 0x30, 0x1, 0x38, 0x1, 0xb1, 0x3, 0xb1, 0x2, 0xbf, 0x0, 0x38, 0x4, 0xb1, 0x5, 0xb1, 0xb, 0x43, 0x0, 0x61, 0x0, 0x6e, 0x0, 0x6f, 0x0, 0x6e, 0x0, 0x20, 0x0, 0x49, 0x0, 0x6e, 0x0, 0x63, 0x0, 0x2e, 0x0, 0x0, 0x0, 0x13, 0x43, 0x0, 0x61, 0x0, 0x6e, 0x0, 0x6f, 0x0, 0x6e, 0x0, 0x20, 0x0, 0x45, 0x0, 0x4f, 0x0, 0x53, 0x0, 0x20, 0x0, 0x52, 0x0, 0x65, 0x0, 0x62, 0x0, 0x65, 0x0, 0x6c, 0x0, 0x20, 0x0, 0x54, 0x0, 0x36, 0x0, 0x0, 0x0, 0x8, 0x33, 0x0, 0x2d, 0x0, 0x31, 0x0, 0x2e, 0x0, 0x31, 0x0, 0x2e, 0x0, 0x30, 0x0, 0x0, 0x0, 0x8, 0x38, 0x0, 0x32, 0x0, 0x38, };

void print_bytes(uint8_t *bytes, int n) {
	for (int i = 0; i < n; i++) {
		if (bytes[i] > 31 && bytes[i] < 128) {
			printf("'%c' ", bytes[i]);
		} else {
			printf("%02X ", bytes[i]);
		}
	}

	puts("");
}

int main() {
	r.data = malloc(4096);
	r.data_length = 4096;

	memcpy(r.data, export, sizeof(export));

	struct PtpDeviceInfo di;

	puts("TESTING GET DEVICE INFO");
	ptp_parse_device_info(&r, &di);
	ptp_device_info_json(&di, (char*)r.data, r.data_length);
	printf("%s\n", (char*)r.data);

	struct PtpCommand cmd;
	cmd.data_length = 50;
	cmd.param_length = 0;
	cmd.code = PTP_OC_Canon_ExecEventProc;

	int length = ptp_bulk_packet_data(&r, &cmd);
	memset(r.data + length, 0, 100);
	int i = ptp_wide_string(r.data + length, 100, "EnableBootDisk");
	printf("%d\n", i);

	print_bytes(r.data, 100);
}

