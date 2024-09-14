// Packet generation, parsing, and manipulation routines
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <ptp.h>
#include <camlib.h>

static void boundcheck(uint8_t *bs, uint8_t *be, int size) {
	if (be == NULL) return;
	if ((uintptr_t)bs > (uintptr_t)be) ptp_panic("PTP: bs after be\n");
	if (((uintptr_t)be - (uintptr_t)bs) < (uintptr_t)size) {
		ptp_panic("PTP: buffer overflow %p-%p (%u)\n", bs, be, size);
	}
}

// Read standard UTF16 string
int ptp_read_string(uint8_t *d, char *string, int max) {
	int of = 0;
	uint8_t length;
	of += ptp_read_u8(d + of, &length);

	uint8_t i = 0;
	while (i < length) {
		uint16_t wchr;
		of += ptp_read_u16(d + of, &wchr);
		if (wchr > 128) wchr = '?';
		else if (wchr != '\0' && wchr < 32) wchr = ' ';
		string[i] = (char)wchr;
		i++;
		if (i >= max) break;
	}

	string[i] = '\0';

	return of;
}

int ptp_read_uint16_array(const uint8_t *dat, uint16_t *buf, int max, int *length) {
	int of = 0;

	uint32_t n;
	of += ptp_read_u32(dat + of, &n);

	for (uint32_t i = 0; i < n; i++) {
		if (i >= max) {
			ptp_panic("ptp_read_uint16_array overflow\n");
		} else {
			of += ptp_read_u16(dat + of, &buf[i]);
		}
	}

	return of;
}

int ptp_read_uint16_array_s(uint8_t *bs, uint8_t *be, uint16_t *buf, int max, int *length) {
	int of = 0;
	uint32_t n;
	of += ptp_read_u32(bs + of, &n);
	(*length) = n;
	boundcheck(bs, be, 4 * n + 4);
	for (uint32_t i = 0; i < n; i++) {
		if (i >= max) {
			ptp_panic("ptp_read_uint16_array overflow %i >= %d\n", i, n);
		} else {
			of += ptp_read_u16(bs + of, &buf[i]);
		}
	}
	return of;	
}

// Write standard PTP wchar string
int ptp_write_string(uint8_t *dat, const char *string) {
	int of = 0;

	uint32_t length = strlen(string);
	of += ptp_write_u8(dat + of, length);

	for (int i = 0; i < length; i++) {
		of += ptp_write_u8(dat + of, string[i]);
		of += ptp_write_u8(dat + of, '\0');
	}

	return of;
}

// Write normal UTF-8 string
int ptp_write_utf8_string(void *dat, const char *string) {
	char *o = (char *)dat;
	int x = 0;
	while (string[x] != '\0') {
		o[x] = string[x];
		x++;
	}

	o[x] = '\0';
	x++;
	return x;
}

// Write null-terminated UTF16 string
int ptp_write_unicode_string(char *dat, const char *string) {
	int i;
	for (i = 0; string[i] != '\0'; i++) {
		dat[i * 2] = string[i];
		dat[i * 2 + 1] = '\0';
	}
	dat[i * 2 + 1] = '\0';
	return i;
}

// Read null-terminated UTF16 string
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

int ptp_read_utf8_string(void *dat, char *string, int max) {
	char *d = (char *)dat;
	int x = 0;
	while (d[x] != '\0') {
		string[x] = d[x];
		x++;
		if (x > max - 1) break;
	}

	string[x] = '\0';
	x++;

	return x;
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

int ptpip_data_start_packet(struct PtpRuntime *r, int data_length) {
	struct PtpIpStartDataPacket *pkt = (struct PtpIpStartDataPacket *)(r->data);
	pkt->length = 0x20;
	pkt->type = PTPIP_DATA_PACKET_START;
	pkt->transaction = r->transaction;
	pkt->data_phase_length = (uint64_t)data_length;	

	return pkt->length;
}

int ptpip_data_end_packet(struct PtpRuntime *r, void *data, int data_length) {
	struct PtpIpEndDataPacket *pkt = (struct PtpIpEndDataPacket *)(r->data);
	pkt->length = 12 + data_length;
	pkt->type = PTPIP_DATA_PACKET_END;
	pkt->transaction = r->transaction;

	memcpy(((uint8_t *)r->data) + 12, data, data_length);

	return pkt->length;
}

// Generate a USB-only BulkContainer packet
int ptpusb_bulk_packet(struct PtpRuntime *r, struct PtpCommand *cmd, int type) {
	if (cmd->param_length > 5) ptp_panic("cmd->param_length more than 5");
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

// Only for PTP_USB or PTP_USB_IP
int ptp_new_data_packet(struct PtpRuntime *r, struct PtpCommand *cmd, void *data, int data_length) {
	cmd->param_length = 0;

	int length = ptpusb_bulk_packet(r, cmd, PTP_PACKET_TYPE_DATA);

	struct PtpBulkContainer *c = (struct PtpBulkContainer *)(r->data);
	c->length += data_length;

	memcpy(r->data + 12, data, data_length);
	
	return length + data_length;
}

// Generate a IP or USB style command packet (both are pretty similar)
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

// PTPIP: Get data start packet, then data end packet, then response packet
static struct PtpIpResponseContainer *ptpip_get_response_packet(struct PtpRuntime *r) {
	struct PtpIpStartDataPacket *ds = (struct PtpIpStartDataPacket*)(r->data);
	if (ds->type == PTPIP_COMMAND_RESPONSE) {
		// If there is no data phase, return first packet
		return (struct PtpIpResponseContainer *)(r->data);
	}

	if (ds->type != PTPIP_DATA_PACKET_START) {
		ptp_panic("ptpip_get_response_packet(): didn't get data start packet");
	}

	struct PtpIpEndDataPacket *de = (struct PtpIpEndDataPacket *)(r->data + ds->length);
	if (de->type != PTPIP_DATA_PACKET_END) {
		ptp_panic("ptpip_get_response_packet(): didn't get data end packet");
	}

	struct PtpIpResponseContainer *resp = (struct PtpIpResponseContainer *)(r->data + ds->length + de->length);
	if (resp->type != PTPIP_COMMAND_RESPONSE) {
		ptp_panic("ptpip_get_response_packet(): didn't get response packet");
	}

	return resp;
}

// Update transid for current request packet
void ptp_update_transaction(struct PtpRuntime *r, int t) {
	if (r->connection_type == PTP_IP) {
		struct PtpIpBulkContainer *bulk = (struct PtpIpBulkContainer *)(r->data);
		bulk->transaction = t;
	} else {
		struct PtpBulkContainer *bulk = (struct PtpBulkContainer *)(r->data);
		bulk->transaction = t;
	}
}

// Get rccode from response packet
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

// Get ptr to payload
uint8_t *ptp_get_payload(struct PtpRuntime *r) {
	if (r->connection_type == PTP_IP) {
		// For IP, payload is in the DATA_END packet
		struct PtpIpStartDataPacket *ds = (struct PtpIpStartDataPacket*)(r->data);
		if (ds->type != PTPIP_DATA_PACKET_START) {
			ptp_panic("ptp_get_payload(): non data start packet");
		}

		struct PtpIpHeader *de = (struct PtpIpHeader*)(r->data + ds->length);
		if (de->type != PTPIP_DATA_PACKET_END) {
			ptp_panic("ptp_get_payload(): non data end packet");
		}

		return r->data + ds->length + 12;
	} else {
		struct PtpBulkContainer *bulk = (struct PtpBulkContainer*)(r->data);
		if (bulk->type == PTP_PACKET_TYPE_RESPONSE) {
			return r->data + 12;
		} else if (bulk->type == PTP_PACKET_TYPE_COMMAND) {
			return r->data + 28;
		} else {
			// TODO: Fatal
			return r->data + 12;
		}
	}
}

int ptp_get_payload_length(struct PtpRuntime *r) {
	if (r->connection_type == PTP_IP) {
		struct PtpIpStartDataPacket *ds = (struct PtpIpStartDataPacket*)(r->data);
		if (ds->type != PTPIP_DATA_PACKET_START) {
			ptp_panic("ptp_get_payload_length(): non data start packet");
		}

		return (int)ds->data_phase_length;
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

int ptp_get_last_transaction_id(struct PtpRuntime *r) {
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
