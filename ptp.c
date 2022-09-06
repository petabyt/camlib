#include <stdint.h>
#include <string.h>

#include "ptp.h"
#include "lib.h"

int ptp_recv_packet(struct PtpRuntime *r, uint16_t code, uint32_t params[5], int param_length, int read_size) {
	struct PtpBulkContainer bulk;
	int size = 12 + (sizeof(uint32_t) * param_length);
	bulk.length = size + read_size;
	bulk.type = PACKET_TYPE_DATA;
	bulk.code = code;
	bulk.transaction = r->transaction;

	bulk.param1 = params[0];
	bulk.param2 = params[1];
	bulk.param3 = params[2];
	bulk.param4 = params[3];
	bulk.param5 = params[4];

	memcpy(r->data, &bulk, size);

	r->transaction++;
	return size;
}

int ptp_recv_packet_pre(struct PtpRuntime *r, uint16_t code) {
	struct PtpBulkContainer bulk;
	bulk.length = 12;
	bulk.type = PACKET_TYPE_COMMAND;
	bulk.code = code;
	bulk.transaction = r->transaction;
	memcpy(r->data, &bulk, 12);
	return 12;
}