// Common IO backend code
// This is incompatible with Win32 - don't link this in for Windows

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>
#include <backend.h>
#include <ptp.h>

int ptp_send_bulk_packets(struct PtpRuntime *r, int length) {
	int sent = 0;
	while (1) {
		int x = ptp_send_bulk_packet(r->data + sent, length);
		if (x < 0 || x == NULL) {
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
		if (x < 0 || x == NULL) {
			PTPLOG("recieve_bulk_packet: %d\n", x);
		}
		read += x;

		if (read >= r->data_length - r->max_packet_size) {
			PTPLOG("recieve_bulk_packets: Not enough memory\n");
			return PTP_OUT_OF_MEM;
		}

		if (x < 0) {
			PTPLOG("recieve_bulk_packets: ptp_bulk_read < 0, IO error\n");
			return PTP_IO_ERR;
		} else if (x != r->max_packet_size) {
			PTPLOG("recieve_bulk_packets: Read %d bytes\n", read);
			struct PtpBulkContainer *c = (struct PtpBulkContainer *)(r->data);

			// Read the response packet if only a data packet was sent
			if (c->type == PTP_PACKET_TYPE_DATA) {
				x = ptp_recieve_bulk_packet(r->data + read, r->max_packet_size);
				PTPLOG("recieve_bulk_packets: Recieved extra packet %d bytes\n", x);
			}

			PTPLOG("recieve_bulk_packets: Return code: 0x%X\n", ptp_get_return_code(r));
			//PTPLOG("recieve_bulk_packets: Param1: 0x%X\n", ptp_get_param(r, 0));

			// No more more packets to read
			return read;
		}
	}
}

int ptp_read_int(struct PtpRuntime *r) {
	int x = ptp_recieve_int(r->data, r->max_packet_size);
	if (x < 0 || x == NULL) {
		return PTP_IO_ERR;
	}

	return x;
}
