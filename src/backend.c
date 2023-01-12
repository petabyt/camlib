// Common IO backend code
// This is incompatible with Win32 - don't link this in for Windows
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>
#include <backend.h>
#include <ptp.h>

int ptp_send_bulk_packets(struct PtpRuntime *r, int length) {
	//PTPLOG("send_bulk_packets 0x%X\n", ptp_get_return_code(r));

	int sent = 0;
	while (1) {
		int x = ptp_send_bulk_packet(r->data + sent, length);
		if (x < 0) {
			PTPLOG("send_bulk_packet: %d\n", x);
			return PTP_IO_ERR;
		}
		
		sent += x;
		
		if (sent >= length) {
			PTPLOG("send_bulk_packet: Sent %d bytes\n", sent);
			return sent;
		}
	}
}

int ptp_recieve_bulk_packets(struct PtpRuntime *r) {
	int read = 0;

	while (1) {
		int x = ptp_recieve_bulk_packet(r->data + read, r->max_packet_size);
		if (x < 0) {
			PTPLOG("recieve_bulk_packet: %d\n", x);
			return PTP_IO_ERR;
		}
		read += x;

		if (read >= r->data_length - r->max_packet_size) {
			PTPLOG("recieve_bulk_packets: Not enough memory\n");
			return PTP_OUT_OF_MEM;
		}

		if (x != r->max_packet_size) {
			PTPLOG("recieve_bulk_packets: Read %d bytes\n", read);
			struct PtpBulkContainer *c = (struct PtpBulkContainer *)(r->data);

			// Read the response packet if only a data packet was sent
			if (c->type == PTP_PACKET_TYPE_DATA) {
				x = ptp_recieve_bulk_packet(r->data + read, r->max_packet_size);
				PTPLOG("recieve_bulk_packets: Recieved extra packet %d bytes\n", x);
			}

			PTPLOG("recieve_bulk_packets: Return code: 0x%X\n", ptp_get_return_code(r));

			return read;
		}
	}
}

int ptp_send_file_packets(struct PtpRuntime *r, int length, FILE *stream) {
	//PTPLOG("send_bulk_packets 0x%X\n", ptp_get_return_code(r));

	int x = ptp_send_bulk_packet(r->data, length);
	if (x < 0) {
		PTPLOG("send_bulk_packet: %d\n", x);
		return PTP_IO_ERR;
	}

	int sent = x;
	
	while (1) {
		x = fread(r->data, 1, r->max_packet_size, stream);
		if (x <= 0) {
			PTPLOG("fread: %d", x);
			return PTP_IO_ERR;
		}

		int x = ptp_send_bulk_packet(r->data, x);
		if (x < 0) {
			PTPLOG("send_bulk_packet: %d\n", x);
			return PTP_IO_ERR;
		}
		
		sent += x;
		
		if (sent >= length) {
			PTPLOG("send_bulk_packet: Sent %d bytes\n", sent);
			return sent;
		}
	}
}

// TODO: add length to skip bytes
int ptp_frecieve_bulk_packets(struct PtpRuntime *r, FILE *stream) {
	int read = 0;

	int type = -1;

	while (1) {
		int x = ptp_recieve_bulk_packet(r->data, r->max_packet_size);
		if (x < 0) {
			PTPLOG("recieve_bulk_packet: %d\n", x);
			return PTP_IO_ERR;
		}

		if (type == -1) {
			struct PtpBulkContainer *c = (struct PtpBulkContainer *)(r->data);
			type = c->type;
		}

		int fr = fwrite(r->data, 1, x, stream);
		if (fr <= 0) {
			PTPLOG("fwrite: %d\n", fr);
		}
		
		read += x;

		if (x != r->max_packet_size) {
			PTPLOG("recieve_bulk_packets: Read %d bytes\n", read);

			// Read the response packet if only a data packet was sent
			if (type == PTP_PACKET_TYPE_DATA) {
				x = ptp_recieve_bulk_packet(r->data, r->max_packet_size);
				PTPLOG("recieve_bulk_packets: Return code: 0x%X\n", ptp_get_return_code(r));
			} else {
				// TODO: Why send a small packet with stream reader?
			}

			return read;
		}
	}	
}

int ptp_read_int(struct PtpRuntime *r) {
	int x = ptp_recieve_int(r->data, r->max_packet_size);
	if (x < 0) {
		return PTP_IO_ERR;
	}

	return x;
}
