// Packet generation, parsing, and manipulation routines
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <ptp.h>
#include <camlib.h>

uint8_t ptp_read_uint8(void **dat) {
	uint8_t **p = (uint8_t **)dat;
	uint8_t x = (**p);
	(*p)++;
	return x;
}

uint16_t ptp_read_uint16(void **dat) {
	uint16_t **p = (uint16_t **)dat;
	uint16_t x = (**p);
	(*p)++;
	return x;
}

uint32_t ptp_read_uint32(void **dat) {
	uint32_t **p = (uint32_t **)dat;
	uint32_t x = (**p);
	(*p)++;
	return x;
}

// Read UTF16 string
void ptp_read_string(void **dat, char *string, int max) {
	uint8_t **p = (uint8_t **)dat;
	int length = (int)ptp_read_uint8((void **)p);

	int y = 0;
	while (y < length) {
		string[y] = (char)(**p);
		(*p)++;
		//printf("'%c'\n", (char)(*p));
		//assert((char)(**dat) == '\0');
		(*p)++;
		y++;
		if (y >= max) { break; }
	}

	string[y] = '\0';
}

int ptp_read_uint16_array(void **dat, uint16_t *buf, int max) {
	int n = ptp_read_uint32((void **)dat);

	// Probably impossbile scenario
	if (n > 0xff) {
		return -1;
	}

	for (int i = 0; i < n; i++) {
		// Give a zero if out of bounds
		if (i >= max) {
			buf[i] = 0;
		} else {
			buf[i] = ptp_read_uint16((void **)dat);
		}
	}

	return n;
}

void ptp_write_uint8(void **dat, uint8_t b) {
	uint8_t **ptr = (uint8_t **)dat;
	(**ptr) = b;
	(*ptr)++;
}

int ptp_write_string(void **dat, char *string) {
	int length = strlen(string);
	ptp_write_uint8(dat, length);

	for (int i = 0; i < length; i++) {
		ptp_write_uint8(dat, string[i]);
		ptp_write_uint8(dat, '\0');
	}

	ptp_write_uint8(dat, '\0');

	return (length * 2) + 2;
}

// Write non-PTP standard unicode string
int ptp_write_unicode_string(char *dat, char *string) {
	int i;
	for (i = 0; string[i] != '\0'; i++) {
		dat[i * 2] = string[i];
		dat[i * 2 + 1] = '\0';
	}
	dat[i * 2 + 1] = '\0';
	return i;
}

int ptp_read_unicode_string(char *buffer, char *dat, int max) {
	int i;
	for (i = 0; dat[i] != '\0'; i += 2) {
		buffer[i / 2] = dat[i];
		if (i >= max) {
			buffer[(i / 2) + 1] = '\0';
			return i;
		}
	}

	buffer[(i / 2)] = '\0';
	return i / 2;
}

// PTP/IP-specific packet
int ptpip_bulk_packet(struct PtpRuntime *r, struct PtpCommand *cmd, int type) {
	struct PtpIpBulkContainer bulk;
	int size = 18 + (sizeof(uint32_t) * cmd->param_length);
	bulk.length = size;
	bulk.type = type;
	bulk.length += cmd->data_length;
	bulk.code = cmd->code;
	bulk.transaction = r->transaction;

	if (r->data_phase_length == 0) {
		bulk.data_phase = 1;
	} else {
		bulk.data_phase = 2;
	}

	for (int i = 0; i < 5; i++) {
		bulk.params[i] = cmd->params[i];
	}

	memcpy(r->data, &bulk, size);

	return size;
}

// Generate a USB-only BulkContainer packet
int ptpusb_bulk_packet(struct PtpRuntime *r, struct PtpCommand *cmd, int type) {
	struct PtpBulkContainer bulk;
	int size = 12 + (sizeof(uint32_t) * cmd->param_length);

	bulk.length = size;
	bulk.type = type;
	bulk.length += cmd->data_length;
	bulk.code = cmd->code;
	bulk.transaction = r->transaction;

	for (int i = 0; i < 5; i++) {
		bulk.params[i] = cmd->params[i];
	}

	memcpy(r->data, &bulk, size);

	return size;
}

int ptp_new_data_packet(struct PtpRuntime *r, struct PtpCommand *cmd) {
	cmd->param_length = 0;
	if (r->connection_type == PTP_IP) {
		// Create PTPIP_DATA_PACKET_START
		// Create PTPIP_DATA_PACKET_END
	} else {
		int length = ptpusb_bulk_packet(r, cmd, PTP_PACKET_TYPE_DATA);
		return length;
	}
}

int ptp_new_cmd_packet(struct PtpRuntime *r, struct PtpCommand *cmd) {
	cmd->data_length = 0;
	if (r->connection_type == PTP_IP) {
		int length = ptpip_bulk_packet(r, cmd, PTPIP_COMMAND_REQUEST);
		return length;
	} else {
		int length = ptpusb_bulk_packet(r, cmd, PTP_PACKET_TYPE_COMMAND);
		return length;
	}
}

void ptp_update_data_length(struct PtpRuntime *r, int length) {
	if (r->connection_type == PTP_IP) {
		struct PtpIpStartDataPacket *ds = (struct PtpIpStartDataPacket*)(r->data);
		if (ds->type != PTPIP_DATA_PACKET_START) {
			exit(1);
		}

		ds->data_phase_length = length;

		struct PtpIpHeader *de = (struct PtpIpHeader*)(r->data + ds->length);
		if (ds->type != PTPIP_DATA_PACKET_END) {
			exit(1);
		}

		// Update the packet length for the end packet
		de->length = 12 + length;
	} else {
		struct PtpBulkContainer *bulk = (struct PtpBulkContainer*)(r->data);
		bulk->length = length;
	}
}

static struct PtpIpResponseContainer *ptpip_get_response_packet(struct PtpRuntime *r) {
	// Get data start packet, then data end packet, then response packet
	struct PtpIpStartDataPacket *ds = (struct PtpIpStartDataPacket*)(r->data);
	if (ds->type == PTPIP_COMMAND_RESPONSE) {
		return (struct PtpIpResponseContainer *)(r->data);
	}

	// TODO: This assumes sanity
	struct PtpIpHeader *de = (struct PtpIpHeader*)(r->data + ds->length);
	struct PtpIpResponseContainer *resp = (struct PtpIpResponseContainer *)(r->data + ds->length + de->length);
	return resp;
}

void ptp_update_transaction(struct PtpRuntime *r, int t) {
	if (r->connection_type == PTP_IP) {
		struct PtpIpBulkContainer *bulk = (struct PtpIpBulkContainer *)(r->data);
		bulk->transaction = t;
	} else {
		struct PtpBulkContainer *bulk = (struct PtpBulkContainer *)(r->data);
		bulk->transaction = t;
	}
}

int ptp_get_return_code(struct PtpRuntime *r) {
	if (r->connection_type == PTP_IP) {
		struct PtpIpResponseContainer *resp = ptpip_get_response_packet(r);
		return resp->code;
	} else {
		struct PtpBulkContainer *bulk = (struct PtpBulkContainer*)(r->data);
		if (bulk->type == PTP_PACKET_TYPE_DATA) {
			bulk = (struct PtpBulkContainer*)(r->data + bulk->length);
			return bulk->code;
		} else {
			return bulk->code;
		}
	}
}

uint8_t *ptp_get_payload(struct PtpRuntime *r) {
	if (r->connection_type == PTP_IP) {
		struct PtpIpStartDataPacket *ds = (struct PtpIpStartDataPacket*)(r->data);
		if (ds->type != PTPIP_DATA_PACKET_START) {
			ptp_verbose_log("Fatal: non data start packet\n", ((int *)NULL)[0]);
			exit(1);
		}

		struct PtpIpHeader *de = (struct PtpIpHeader*)(r->data + ds->length);
		if (de->type != PTPIP_DATA_PACKET_END) {
			ptp_verbose_log("Fatal: non data end packet, got %X\n", de->type);
			exit(1);
		}

		return r->data + ds->length + 12;
	} else {
		struct PtpBulkContainer *bulk = (struct PtpBulkContainer*)(r->data);
		if (bulk->type == PTP_PACKET_TYPE_RESPONSE) {
			return r->data + 12;
		} else if (bulk->type == PTP_PACKET_TYPE_COMMAND) {
			return r->data + 28;
		} else {
			return r->data + 12;
		}
	}
}

int ptp_get_payload_length(struct PtpRuntime *r) {
	if (r->connection_type == PTP_IP) {
		struct PtpIpStartDataPacket *ds = (struct PtpIpStartDataPacket*)(r->data);
		if (ds->type != PTPIP_DATA_PACKET_START) {
			exit(1);
		}

		return ds->data_phase_length;
	} else {
		struct PtpBulkContainer *bulk = (struct PtpBulkContainer*)(r->data);
		return bulk->length - 12;
	}
}

int ptp_get_param_length(struct PtpRuntime *r) {
	if (r->connection_type == PTP_IP) {
		struct PtpIpResponseContainer *resp = ptpip_get_response_packet(r);	
		return (resp->length - 14) / 4;	
	} else {
		struct PtpBulkContainer *bulk = (struct PtpBulkContainer*)(r->data);

		// Get response packet, which backend stores after data packet
		if (bulk->type == PTP_PACKET_TYPE_DATA) {
			bulk = (struct PtpBulkContainer*)(r->data + bulk->length);
		}

		return (bulk->length - 12) / 4;
	}
}

uint32_t ptp_get_param(struct PtpRuntime *r, int index) {
	if (r->connection_type == PTP_IP) {
		struct PtpIpResponseContainer *resp = ptpip_get_response_packet(r);	
		return resp->params[index];		
	} else {
		struct PtpBulkContainer *bulk = (struct PtpBulkContainer*)(r->data);

		// Get response packet, which backend stores after data packet
		if (bulk->type == PTP_PACKET_TYPE_DATA) {
			bulk = (struct PtpBulkContainer*)(r->data + bulk->length);
			return bulk->code;
		}

		return bulk->params[index];
	}
}

int ptp_get_last_transaction(struct PtpRuntime *r) {
	if (r->connection_type == PTP_IP) {
		struct PtpIpResponseContainer *resp = ptpip_get_response_packet(r);	
		return resp->transaction;		
	} else {
		struct PtpBulkContainer *bulk = (struct PtpBulkContainer*)(r->data);
		if (bulk->type == PTP_PACKET_TYPE_DATA) {
			bulk = (struct PtpBulkContainer*)(r->data + bulk->length);
			return bulk->transaction;
		} else {
			return bulk->transaction;
		}
	}
}
