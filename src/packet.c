#include <stdint.h>
#include <string.h>

#include "ptp.h"
#include "camlib.h"

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

int ptp_read_uint32_array(void **dat, uint16_t *buf, int max) {
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
			buf[i] = ptp_read_uint32(dat);
		}
	}

	return n;
}

int ptp_wide_string(char *buffer, int max, char *input) {
	int i;
	for (i = 0; (i < max) && input[i] != '\0'; i++) {
		buffer[i * 2] = input[i];
		buffer[i * 2 + 1] = 0; 
	}

	return i * 2 + 1;
}

// Generate a BulkContainer data packet to recieve or send data
int ptp_bulk_packet_data(struct PtpRuntime *r, struct PtpCommand *cmd) {
	struct PtpBulkContainer bulk;
	int size = 12 + (sizeof(uint32_t) * cmd->param_length);
	bulk.length = size + cmd->data_length;
	bulk.type = PACKET_TYPE_DATA;
	bulk.code = cmd->code;
	bulk.transaction = r->transaction;

	bulk.param1 = cmd->params[0];
	bulk.param2 = cmd->params[1];
	bulk.param3 = cmd->params[2];
	bulk.param4 = cmd->params[3];
	bulk.param5 = cmd->params[4];

	memcpy(r->data, &bulk, size);

	r->transaction++;
	return size;
}

// Generate a "command" packet that is sent before a data packet
int ptp_bulk_packet_cmd(struct PtpRuntime *r, struct PtpCommand *cmd) {
	struct PtpBulkContainer bulk;
	bulk.length = 12; // standard bulk length with 0 parameters
	bulk.type = PACKET_TYPE_COMMAND;
	bulk.code = cmd->code;
	bulk.transaction = r->transaction;
	memcpy(r->data, &bulk, bulk.length);
	return bulk.length;
}
