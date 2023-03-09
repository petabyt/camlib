// Common IO backend code - only applies to platforms that have generic
// USB packet IO access

// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>
#include <ptp.h>

int ptp_send_bulk_packets(struct PtpRuntime *r, int length) {
	PTPLOG("send_bulk_packets 0x%X (%s)\n", ptp_get_return_code(r), ptp_get_enum_all(ptp_get_return_code(r)));

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
			// Check if first time reading, try again once
			if (read == 0) {
				PTPLOG("Failed to recieve packet, trying again...\n");
				CAMLIB_SLEEP(100);
				x = ptp_recieve_bulk_packet(r->data + read, r->max_packet_size);
			}

			if (x < 0) {
				PTPLOG("recieve_bulk_packet: %d\n", x);
				return PTP_IO_ERR;
			}
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

int ptp_fsend_packets(struct PtpRuntime *r, int length, FILE *stream) {
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

int ptp_frecieve_bulk_packets(struct PtpRuntime *r, FILE *stream, int of) {
	int read = 0;

	// Since the data is written to file, we must remember the packet type
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

		int fr = fwrite(r->data + of, 1, x - of, stream);
		of = 0;
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
