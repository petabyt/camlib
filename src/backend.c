// Common IO backend code - only applies to platforms that have generic
// USB packet IO access (Not Windows)

// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>

// Sending data out is much easier, can be a portable implementation
// TODO: This is named wrong, should be 'ptp_send_packet', only sends single packet
int ptp_send_bulk_packets(struct PtpRuntime *r, int length) {
	int sent = 0;
	int x;
	while (1) {
		if (r->connection_type == PTP_USB) {
			x = ptp_cmd_write(r, r->data + sent, length);
		} else if (r->connection_type == PTP_IP || r->connection_type == PTP_IP_USB) {
			x = ptpip_cmd_write(r, r->data + sent, length);
		}

		if (x < 0) {
			ptp_verbose_log("send_bulk_packet: %d\n", x);
			return PTP_IO_ERR;
		}
		
		sent += x;
		
		if (sent >= length) {
			ptp_verbose_log("send_bulk_packet: Sent %d bytes\n", sent);
			return sent;
		}
	}
}

int ptpip_read_packet(struct PtpRuntime *r, int of) {
	int read = 0;

	int rc = ptpip_cmd_read(r, r->data + of, 8);
	if (rc != 8) {
		ptp_verbose_log("Failed to read PTP/IP header\n");
		return PTP_IO_ERR;
	}

	read += rc;

	struct PtpIpHeader *h = (struct PtpIpHeader *)(r->data + of);

	printf("Got packet of %d bytes, type %X\n", h->length, h->type);

	while (1) {
		rc = ptpip_cmd_read(r, r->data + of + read, h->length - read);
		if (rc < 0) {
			return PTP_IO_ERR;
		}

		read += rc;

		if (h->length - read == 0) {
			return read;
		}
	}
}

int ptpip_receive_bulk_packets(struct PtpRuntime *r) {
	int rc = ptpip_read_packet(r, 0);
	if (rc < 0) {
		return rc;
	}

	int pk1_of = rc;

	struct PtpIpHeader *h = (struct PtpIpHeader *)(r->data);

	if (h->type == PTPIP_DATA_PACKET_START) {
		rc = ptpip_read_packet(r, pk1_of);
		h = (struct PtpIpHeader *)(r->data + pk1_of);
		if (h->type != PTPIP_DATA_PACKET_END) {
			ptp_verbose_log("Didn't receive an END DATA packet (%d)\n", h->type);
			return PTP_IO_ERR;
		}

		int pk2_of = pk1_of + h->length;

		rc = ptpip_read_packet(r, pk2_of);
		h = (struct PtpIpHeader *)(r->data + pk2_of);
		if (h->type != PTPIP_COMMAND_RESPONSE) {
			printf("%d\n", h->length);
			ptp_verbose_log("Non response packet after data end packet (%d)\n", h->type);
			return PTP_IO_ERR;
		}
	} else if (h->type == PTPIP_COMMAND_RESPONSE) {
		ptp_verbose_log("Recieved response packet\n");
	} else {
		ptp_verbose_log("Unexpected packet: %X\n", h->type);
		return PTP_IO_ERR;
	}

//	ptp_verbose_log("receive_bulk_packets: Read %d bytes\n", read);
//	ptp_verbose_log("receive_bulk_packets: Return code: 0x%X\n", ptp_get_return_code(r));

	return 0;
}

int ptpip_write_packet(struct PtpRuntime *r, int of) {
	struct PtpIpHeader *h = (struct PtpIpHeader *)(r->data + of);

	int rc = ptpip_cmd_write(r, r->data + of, h->length);
	if (rc < 0) {
		return PTP_IO_ERR;
	} else if (rc != h->length) {
		return PTP_IO_ERR;
	}

	return rc;
}

int ptpusb_read_packet(struct PtpRuntime *r, int of) {
	int rc = 0;
	int read = 0;

	while (rc <= 0 && r->wait_for_response) {
		if (r->connection_type == PTP_USB) {
			rc = ptp_cmd_read(r, r->data + of + read, r->max_packet_size);
		} else if (r->connection_type == PTP_IP_USB) {
			rc = ptpip_cmd_read(r, r->data + of + read, 4);
		}

		r->wait_for_response--;

		if (r->wait_for_response) {
			CAMLIB_SLEEP(1000);
		}
	}

	r->wait_for_response = 1;

	if (rc < 0) {
		ptp_verbose_log("USB Read error: %d\n", rc);
		return PTP_IO_ERR;
	}

	if (rc < 4) {
		ptp_verbose_log("Failed to read at least packet length: %d\n", rc);
		return PTP_IO_ERR;
	}

	read += rc;

	struct PtpBulkContainer *h = (struct PtpBulkContainer *)(r->data + of);

	if (h->length - read == 0) {
		return read;
	}

	// Read more than a single packet, caller must handle this
	if (h->length < read) {
		// Read too many bytes. Only case when this would happen is when data packet and
		// response packet are read as one.
		int extra = read - h->length;

		// If packet is completely read in, then we are good to go
		if (extra >= 4) {
			struct PtpBulkContainer *h2 = (struct PtpBulkContainer *)(r->data + of + read);
			if (h2->length == extra) return read;
		}
		
		// If not, then reading r->max_packet_size will always fix this (assuming r->max_packet_size >= 512)
		rc = ptp_cmd_read(r, r->data + of + read, r->max_packet_size);
		read += rc;
		if (rc < 0) {
			ptp_verbose_log("USB Read error: %d\n", rc);
			return PTP_IO_ERR;
		}

		return read;
	}

	// Ensure data buffer is large enough for the rest of the packet
	if (of + read + h->length >= r->data_length) {
		ptp_verbose_log("Extending IO buffer\n");
		r->data = realloc(r->data, of + read + h->length + 1000);
		r->data_length = of + read + h->length + 1000;
		if (r->data == NULL) {
			return PTP_OUT_OF_MEM;
		}
	}

	while (1) {
		if (r->connection_type == PTP_USB) {
			rc = ptp_cmd_read(r, r->data + of + read, h->length - read);
		} else if (r->connection_type == PTP_IP_USB) {
			rc = ptpip_cmd_read(r, r->data + of + read, h->length - read);
		}

		if (rc < 0) {
			ptp_verbose_log("USB Read error: %d\n", rc);
			return PTP_IO_ERR;
		}

		read += rc;

		if (h->length - read == 0) {
			return read;
		}
	}
}

int ptpusb_receive_bulk_packets(struct PtpRuntime *r) {
	int read = 0;
	while (1) {
		int rc = ptpusb_read_packet(r, read);
		if (rc < 0) {
			return rc;
		}

		struct PtpBulkContainer *c = (struct PtpBulkContainer *)(r->data + read);
		if (c->length < rc) {
			ptp_verbose_log("Already read enough bytes\n");
			return read;
		}

		read += rc;

		// Handle data phase
		if (c->type == PTP_PACKET_TYPE_DATA) {
			rc = ptpusb_read_packet(r, read);
			if (rc < 0) {
				return rc;
			}

			read += rc;
		}

		ptp_verbose_log("receive_bulk_packets: Read %d bytes\n", read);
		ptp_verbose_log("receive_bulk_packets: Return code: 0x%X\n", ptp_get_return_code(r));

		return read;
	}
}

int ptp_receive_bulk_packets(struct PtpRuntime *r) {
	if (r->connection_type == PTP_IP) {
		return ptpip_receive_bulk_packets(r);
	} else {
		return ptpusb_receive_bulk_packets(r);
	}
}





// Pipe-routing IO, untested, don't use yet
int ptp_fsend_packets(struct PtpRuntime *r, int length, FILE *stream) {
	int x = ptp_cmd_write(r, r->data, length);
	if (x < 0) {
		ptp_verbose_log("send_bulk_packet: %d\n", x);
		return PTP_IO_ERR;
	}

	int sent = x;
	
	while (1) {
		x = fread(r->data, 1, r->max_packet_size, stream);
		if (x <= 0) {
			ptp_verbose_log("fread: %d", x);
			return PTP_IO_ERR;
		}

		if (r->connection_type == PTP_USB) {
			x = ptp_cmd_write(r, r->data, x);
		} else if (r->connection_type == PTP_IP || r->connection_type == PTP_IP_USB) {
			x = ptpip_cmd_write(r, r->data, x);
		}

		if (x < 0) {
			ptp_verbose_log("send_bulk_packet: %d\n", x);
			return PTP_IO_ERR;
		}
		
		sent += x;
		
		if (sent >= length) {
			ptp_verbose_log("send_bulk_packet: Sent %d bytes\n", sent);
			return sent;
		}
	}
}

// TODO: Fix for IP
int ptp_freceive_bulk_packets(struct PtpRuntime *r, FILE *stream, int of) {
	int read = 0;

	// Since the data is written to file, we must remember the packet type
	int type = -1;
	while (1) {
		int x = ptp_cmd_read(r, r->data, r->max_packet_size);
		if (x < 0) {
			ptp_verbose_log("receive_bulk_packet: %d\n", x);
			return PTP_IO_ERR;
		}

		if (type == -1) {
			struct PtpBulkContainer *c = (struct PtpBulkContainer *)(r->data);
			type = c->type;
		}

		int fr = fwrite(r->data + of, 1, x - of, stream);
		of = 0;
		if (fr <= 0) {
			ptp_verbose_log("fwrite: %d\n", fr);
		}
		
		read += x;

		if (x != r->max_packet_size) {
			ptp_verbose_log("receive_bulk_packets: Read %d bytes\n", read);

			// Read the response packet if only a data packet was sent
			if (type == PTP_PACKET_TYPE_DATA) {
				x = ptp_cmd_read(r, r->data, r->max_packet_size);
				ptp_verbose_log("receive_bulk_packets: Return code: 0x%X\n", ptp_get_return_code(r));
			} else {
				// TODO: Why send a small packet with stream reader?
			}

			return read;
		}
	}
}
