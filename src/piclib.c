#include <stdint.h>
#include <string.h>

#include "ptp.h"
#include "piclib.h"

uint8_t ptp_read_uint8(void **dat) {
	return *((uint8_t*)(dat[0]++));
}

uint16_t ptp_read_uint16(void **dat) {
	uint16_t x = *((uint16_t*)(dat[0]));
	dat[0] += 2;
	return x;
}

uint16_t ptp_read_uint32(void **dat) {
	uint32_t x = *((uint32_t*)(dat[0]));
	dat[0] += 4;
	return x;
}

// Read UTF16 string
void ptp_read_string(void **dat, char *string, int max) {
	int length = (int)ptp_read_uint8(dat);

	int y = 0;
	while (y < length) {
		string[y] = *((char*)dat[0]);
		dat[0] += 2;
		y++;
		if (y >= max) {break;}
	}

	string[y] = '\0';
}

int ptp_read_uint16_array(void **dat, uint16_t *buf, int max) {
	int n = ptp_read_uint32(dat);

	// Probably impossbile scenario
	if (n > 0xff) {
		return -1;
	}

	for (int i = 0; i < n; i++) {
		// Give a zero if out of bounds
		if (i >= max) {
			buf[i] = 0;
		} else {
			buf[i] = ptp_read_uint16(dat);
		}
	}

	return n;
}

// Generate a BulkContainer data packet to recieve data
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

// Generate a "command" packet that is sent before a data packet
int ptp_recv_packet_pre(struct PtpRuntime *r, uint16_t code) {
	struct PtpBulkContainer bulk;
	bulk.length = 12;
	bulk.type = PACKET_TYPE_COMMAND;
	bulk.code = code;
	bulk.transaction = r->transaction;
	memcpy(r->data, &bulk, bulk.length);
	return bulk.length;
}
