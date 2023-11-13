// Packet stream read/write logic - uses bare I/O operations
// Windows doesn't have that, so it can't be used here.

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
		
		if (sent > length) {
			ptp_verbose_log("send_bulk_packet: Sent too many bytes: %d\n", sent);
			return sent;
		} else if (sent == length) {
			ptp_verbose_log("send_bulk_packet: Sent %d/%d bytes\n", sent, length);
			return sent;			
		}
	}
}

int ptpip_read_packet(struct PtpRuntime *r, int of) {
	int rc = 0;
	int read = 0;

	while (rc <= 0 && r->wait_for_response) {
		rc = ptpip_cmd_read(r, r->data + of + read, 4);

		r->wait_for_response--;

		if (rc > 0) break;

		if (r->wait_for_response) {
			ptp_verbose_log("Trying again...");
			CAMLIB_SLEEP(CAMLIB_WAIT_MS);
		}
	}

	r->wait_for_response = 1;

	if (rc < 0) {
		ptp_verbose_log("Failed to read packet length: %d\n", rc);
		return PTP_IO_ERR;
	}

	if (rc < 4) {
		ptp_verbose_log("Failed to read at least packet length: %d\n", rc);
		return PTP_IO_ERR;
	}

	read += rc;

	struct PtpIpHeader *h = (struct PtpIpHeader *)(r->data + of);

	if (h->length - read == 0) {
		return read;
	}

	// Ensure data buffer is large enough for the rest of the packet
	if (of + read + h->length >= r->data_length) {
		rc = ptp_buffer_resize(r, of + read + h->length);
		if (rc) return rc;
	}

	while (1) {
		rc = ptpip_cmd_read(r, r->data + of + read, h->length - read);

		if (rc < 0) {
			ptp_verbose_log("Read error: %d\n", rc);
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
			ptp_verbose_log("Non response packet after data end packet (%d)\n", h->type);
			return PTP_IO_ERR;
		}
	} else if (h->type == PTPIP_COMMAND_RESPONSE) {
		ptp_verbose_log("Recieved response packet\n");
	} else {
		ptp_verbose_log("Unexpected packet: %X\n", h->type);
		return PTP_IO_ERR;
	}

	ptp_verbose_log("receive_bulk_packets: Read %d bytes\n", read);
	ptp_verbose_log("receive_bulk_packets: Return code: 0x%X\n", ptp_get_return_code(r));

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

// Quirk of LibUSB/LibWPD - we can allowed read 512 bytes over and over again
// until we don't, then packet is over. This makes the code simpler and gives a reduces
// calls to the backend, which increases performance. This isn't possible with sockets - 
// the read will time out in most cases.
int ptpusb_read_all_packets(struct PtpRuntime *r) {
	int read = 0;

	while (1) {
		int rc = 0;
		while (rc <= 0 && r->wait_for_response) {
			rc = ptp_cmd_read(r, r->data + read, r->max_packet_size);

			r->wait_for_response--;

			if (rc > 0) break;

			if (r->wait_for_response) {
				ptp_verbose_log("Trying again...");
				CAMLIB_SLEEP(CAMLIB_WAIT_MS);
			}
		}
		r->wait_for_response = 1;

		if (rc < 0) {
			ptp_verbose_log("Failed to read packets: %d\n");
			return PTP_IO_ERR;
		}

		read += rc;

		// TODO: unsigned/unsigned compare
		if (read >= r->data_length - r->max_packet_size) {
			ptp_verbose_log("recieve_bulk_packets: Not enough memory\n");
			return PTP_OUT_OF_MEM;
		}

		if (rc != r->max_packet_size) {
			ptp_verbose_log("recieve_bulk_packets: Read %d bytes\n", read);
			struct PtpBulkContainer *c = (struct PtpBulkContainer *)(r->data);

			// Read the response packet if only a data packet was sent (as per spec, always is 12 bytes)
			if (c->type == PTP_PACKET_TYPE_DATA) {
				rc = ptp_cmd_read(r, r->data + read, r->max_packet_size);
				ptp_verbose_log("recieve_bulk_packets: Recieved response packet: %d\n", rc);
				read += rc;
			}

			ptp_verbose_log("recieve_bulk_packets: Return code: 0x%X\n", ptp_get_return_code(r));

			return read;
		}
	}
}

// For USB packets over IP, we can't do any LibUSB/LibWPD performance tricks. reads will just time out.
int ptpipusb_read_packet(struct PtpRuntime *r, int of) {
	int rc = 0;
	int read = 0;

	while (rc <= 0 && r->wait_for_response) {
		rc = ptpip_cmd_read(r, r->data + of + read, 4);

		r->wait_for_response--;

		if (rc > 0) break;

		if (r->wait_for_response) {
			ptp_verbose_log("Trying again...");
			CAMLIB_SLEEP(CAMLIB_WAIT_MS);
		}
	}

	r->wait_for_response = 1;

	if (rc < 0) {
		ptp_verbose_log("Failed to read packet length: %d\n", rc);
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

	// Ensure data buffer is large enough for the rest of the packet
	if (of + read + h->length >= r->data_length) {
		rc = ptp_buffer_resize(r, of + read + h->length);
		if (rc) return rc;
	}

	while (1) {
		rc = ptpip_cmd_read(r, r->data + of + read, h->length - read);

		if (rc < 0) {
			ptp_verbose_log("Read error: %d\n", rc);
			return PTP_IO_ERR;
		}

		read += rc;

		if (h->length - read == 0) {
			return read;
		}
	}
}

// For reading USB style packets over sockets
int ptpipusb_receive_bulk_packets(struct PtpRuntime *r) {
	int read = 0;
	int rc = 0;

	rc = ptpipusb_read_packet(r, read);
	if (rc < 0) return rc;

	struct PtpBulkContainer *c = (struct PtpBulkContainer *)(r->data + read);

	if (c->length < rc) {
		ptp_verbose_log("Already read enough bytes\n");
		return read;
	}

	read += rc;

	// Handle data phase
	if (c->type == PTP_PACKET_TYPE_DATA) {
		rc = ptpipusb_read_packet(r, read);
		if (rc < 0) return rc;

		read += rc;
	}

	ptp_verbose_log("receive_bulk_packets: Read %d bytes\n", read);
	ptp_verbose_log("receive_bulk_packets: Return code: 0x%X\n", ptp_get_return_code(r));

	return read;
}

int ptp_receive_bulk_packets(struct PtpRuntime *r) {
	if (r->connection_type == PTP_IP) {
		return ptpip_receive_bulk_packets(r);
	} else if (r->connection_type == PTP_USB) {
		return ptpusb_read_all_packets(r);
	} else if (r->connection_type == PTP_IP_USB) {
		return ptpipusb_receive_bulk_packets(r);
	} else {
		return PTP_IO_ERR;
	}
}

#if 0
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
#endif
