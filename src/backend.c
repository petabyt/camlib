// Common IO backend code

#include <camlib.h>
#include <ptp.h>
#include <backend.h>

int ptp_send_bulk_packets(struct PtpRuntime *r, int length) {
	int sent = 0;
	while (1) {
		int x = ptp_send_bulk_packet(r->data + sent, length);
		PTPLOG("send_bulk_packets: usb_bulk_write(%d, %d)\n", x, ptp_backend.endpoint_out);
		if (x < 0) {
			perror("send_bulk_packets:");
			return PTP_IO_ERR;
		}
		
		sent += x;
		
		if (sent >= length) {
			return sent;
		}
	}
}

int ptp_recieve_bulk_packets(struct PtpRuntime *r) {
	int read = 0;

	while (1) {
		int x = ptp_send_bulk_packet(r->data + sent, r->max_packet_size);
		read += x;

		if (read >= r->data_length - r->max_packet_size) {
			PTPLOG("recieve_bulk_packets: Not enough memory\n");
			return PTP_OUT_OF_MEM;
		}

		if (x < 0) {
			PTPLOG("recieve_bulk_packets: ptp_bulk_read < 0, IO error\n");
			return PTP_IO_ERR;
		} else if (x != r->max_packet_size) {
			PTPLOG("recieve_bulk_packets: Read %d bytes\n");
			struct PtpBulkContainer *c = (struct PtpBulkContainer *)(r->data);

			// Read the response packet if only a data packet was sent
			if (c->type == PTP_PACKET_TYPE_DATA) {
				x = usb_bulk_read(ptp_backend.devh,
					ptp_backend.endpoint_in,
					(char*)r->data + read,
				r->max_packet_size, PTP_TIMEOUT);
				PTPLOG("recieve_bulk_packets: Recieved extra packet %d bytes\n", x);
			}

			PTPLOG("recieve_bulk_packets: Return code: %d\n", ptp_get_return_code(r));

			// No more more packets to read
			return read;
		}
	}
}
