#ifndef PTP_LIB_H
#define PTP_LIB_H

#include <stdint.h>

struct PtpRuntime {
	int transaction;
    uint8_t *data;
};

// Generic command structure - order of data isn't important.
// Meant to be a bridge to the other packet types.
struct PtpCommand {
	int code;

	int params[5];
	int param_length;

	uint8_t *data;
	int data_length;
};

int ptp_recv_packet(struct PtpRuntime *r, uint16_t code, uint32_t params[5], int param_length, int read_size);
int ptp_recv_packet_pre(struct PtpRuntime *r, uint16_t code);

#endif