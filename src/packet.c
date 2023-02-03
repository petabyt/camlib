// Packet generation, parsing, and manipulation routines
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)

#include <stdint.h>
#include <string.h>

#include <ptp.h>
#include <camlib.h>

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

void ptp_write_uint8(void **dat, uint8_t b) {
	*((uint8_t*)(dat[0]++)) = b;
}

void ptp_write_string(void **dat, char *string) {
	int length = strlen(string);
	ptp_write_uint8(dat, length);

	for (int i = 0; i < length; i++) {
		ptp_write_uint8(dat, string[i]);
		dat[0] += 2;
	}
}

// Generate a BulkContainer packet
int ptp_bulk_packet(struct PtpRuntime *r, struct PtpCommand *cmd, struct PtpBulkContainer *bulk, int type) {
	int size = 12 + (sizeof(uint32_t) * cmd->param_length);
	bulk->type = type;
	bulk->length = size;
	bulk->length += cmd->data_length;
	bulk->code = cmd->code;
	bulk->transaction = r->transaction;

	for (int i = 0; i < 5; i++) {
		bulk->params[i] = cmd->params[i];
	}

	memcpy(r->data, bulk, size);

	// TODO: Should this line move?
	r->transaction++;

	return size;
}

// Generate a data container packet
int ptp_new_data_packet(struct PtpRuntime *r, struct PtpCommand *cmd) {
	struct PtpBulkContainer bulk;
	int length = ptp_bulk_packet(r, cmd, &bulk, PTP_PACKET_TYPE_DATA);
	return length;
}

// Generate a short "command" container packet
// Page 281 of MTP 1.1 spec
int ptp_new_cmd_packet(struct PtpRuntime *r, struct PtpCommand *cmd) {
	struct PtpBulkContainer bulk;
	cmd->data_length = 0;
	int length = ptp_bulk_packet(r, cmd, &bulk, PTP_PACKET_TYPE_COMMAND);
	return length;
}

// TODO: Sometimes 12 is not the exact size? Maybe ptp_get_payload_offset
int ptp_get_data_length(struct PtpRuntime *r) {
	struct PtpBulkContainer *bulk = (struct PtpBulkContainer*)(r->data);
	return bulk->length - 12;
}

void ptp_update_data_length(struct PtpRuntime *r, int length) {
	struct PtpBulkContainer *bulk = (struct PtpBulkContainer*)(r->data);
	bulk->length = length;
}

void ptp_update_transaction(struct PtpRuntime *r, int t) {
	struct PtpBulkContainer *bulk = (struct PtpBulkContainer*)(r->data);
	bulk->transaction = t;
}

int ptp_get_return_code(struct PtpRuntime *r) {
	struct PtpBulkContainer *bulk = (struct PtpBulkContainer*)(r->data);
	if (bulk->type == PTP_PACKET_TYPE_DATA) {
		bulk = (struct PtpBulkContainer*)(r->data + bulk->length);
		return bulk->code;
	} else {
		return bulk->code;
	}
}

uint8_t *ptp_get_payload(struct PtpRuntime *r) {
	struct PtpBulkContainer *bulk = (struct PtpBulkContainer*)(r->data);
	if (bulk->type == PTP_PACKET_TYPE_RESPONSE) {
		// TODO: return NULL?
		return r->data;
	} else {
		return r->data + 12;
	}
}

int ptp_get_payload_length(struct PtpRuntime *r) {
	struct PtpBulkContainer *bulk = (struct PtpBulkContainer*)(r->data);
	return bulk->length - 12;
}

int ptp_get_param_length(struct PtpRuntime *r) {
	struct PtpBulkContainer *bulk = (struct PtpBulkContainer*)(r->data);

	// Get response packet, which backend stores after data packet
	if (bulk->type == PTP_PACKET_TYPE_DATA) {
		bulk = (struct PtpBulkContainer*)(r->data + bulk->length);
	}

	return (bulk->length - 12) / 4;

	return 0;
}

uint32_t ptp_get_param(struct PtpRuntime *r, int index) {
	struct PtpBulkContainer *bulk = (struct PtpBulkContainer*)(r->data);

	// Get response packet, which backend stores after data packet
	if (bulk->type == PTP_PACKET_TYPE_DATA) {
		bulk = (struct PtpBulkContainer*)(r->data + bulk->length);
		return bulk->code;
	} else {
		return bulk->code;
	}

	return bulk->params[index];

	return 0;
}

int ptp_get_last_transaction(struct PtpRuntime *r) {
	struct PtpBulkContainer *bulk = (struct PtpBulkContainer*)(r->data);
	if (bulk->type == PTP_PACKET_TYPE_DATA) {
		bulk = (struct PtpBulkContainer*)(r->data + bulk->length);
		return bulk->transaction;
	} else {
		return bulk->transaction;
	}
}
