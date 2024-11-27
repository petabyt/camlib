// Packet transport interface for PTPUSB, PTPIP, and PTPUSBIP - uses OS IO functions
// For Windows MTP, this calls functions in libwpd.c
// Copyright 2024 by Daniel C (https://github.com/petabyt/camlib)

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>

/*
Rewrite of backend

ptpip_read_packets:
	ptpip_read_packet()

ptpusb_read_all_packets():
	while read(512) != 512
	ptpusb_check_packets()

ptpusbip_read_all_packets():
	while read(512) != 512
	ptpusb_check_packets()
*/

int ptpusb_send_bulk_packets(struct PtpRuntime *r, int length) {
	int sent = 0;
	int x;
	while (1) {
		x = ptp_cmd_write(r, r->data + sent, length);

		if (x < 0) {
			ptp_verbose_log("%s: %d\n", __func__, x);
			return PTP_IO_ERR;
		}
		
		sent += x;
		
		if (sent > length) {
			ptp_verbose_log("%s: Sent too many bytes: %d\n", __func__, sent);
			return sent;
		} else if (sent == length) {
			ptp_verbose_log("%s: Sent %d/%d bytes\n", __func__, sent, length);
			return sent;			
		}
	}
}

int ptp_send_packet(struct PtpRuntime *r, int length) {
	if (r->connection_type == PTP_USB) {
		return ptpusb_send_bulk_packets(r, length);
	}
	int sent = 0;
	while (1) {
		int x = ptpip_cmd_write(r, r->data + sent, length);

		if (x < 0) {
			ptp_verbose_log("%s: %d\n", __func__, x);
			return PTP_IO_ERR;
		}
		
		sent += x;
		
		if (sent > length) {
			ptp_verbose_log("%s: Sent too many bytes: %d\n", __func__, sent);
			return sent;
		} else if (sent == length) {
			ptp_verbose_log("%s: Sent %d/%d bytes\n", __func__, sent, length);
			return sent;			
		}
	}
}

int ptpip_read_packet(struct PtpRuntime *r, int of) {
	int rc = 0;
	int read = 0;

	while (r->wait_for_response) {
		rc = ptpip_cmd_read(r, r->data + of + read, 4);

		r->wait_for_response--;

		if (rc > 0) break;

		if (r->wait_for_response) {
			ptp_verbose_log("Trying again...\n");
			CAMLIB_SLEEP(CAMLIB_WAIT_MS);
		}
	}

	r->wait_for_response = r->response_wait_default;

	if (rc < 0) {
		ptp_verbose_log("Failed to read packet length: %d\n", rc);
		return PTP_COMMAND_IGNORED;
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

	h = (struct PtpIpHeader *)(r->data + of);

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
		ptp_verbose_log("Received response packet\n");
	} else {
		ptp_verbose_log("Unexpected packet: %X\n", h->type);
		return PTP_IO_ERR;
	}

	ptp_verbose_log("ptpip_receive_bulk_packets: Return code: 0x%X\n", ptp_get_return_code(r));

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

// MTP clients must support 512 byte bulk transactions, so this makes transmission simpler. And a little faster.
int ptpusb_read_all_packets(struct PtpRuntime *r) {
	int read = 0;

	//int max_packet_size = r->max_packet_size;

	int rc = 0;
	while (r->wait_for_response) {
		rc = ptp_cmd_read(r, r->data + read, r->max_packet_size);

		r->wait_for_response--;

		if (rc > 0) break;

		if (r->wait_for_response) {
			ptp_verbose_log("Trying again...\n");
			CAMLIB_SLEEP(CAMLIB_WAIT_MS);
		}
	}
	r->wait_for_response = r->response_wait_default;

	if (rc < 0) {
		ptp_verbose_log("Failed to read packets: %d\n", rc);
		return PTP_IO_ERR;
	}

	read += rc;

	if (read < 12) {
		ptp_verbose_log("Couldn't get min packet size: %d\n", rc);
		return PTP_IO_ERR;
	}

	struct PtpBulkContainer *c = (struct PtpBulkContainer *)(r->data);

	// Response packet is read
	if (c->type == PTP_PACKET_TYPE_RESPONSE) {
		if (read == c->length) {
			return 0;
		} else {
			ptp_verbose_log("Read too much of packet: %d\n", read);
			return PTP_IO_ERR;
		}
	} else {
		// Reallocate enough to fill the rest of data and response packet
		if (r->data_length < (c->length + r->max_packet_size)) {
			rc = ptp_buffer_resize(r, c->length + r->max_packet_size);
			if (rc) return rc;
		}

		// Make sure to read all of data packet
		while (read <= c->length) {
			rc = ptp_cmd_read(r, r->data + read, r->max_packet_size);
			if (rc < 0) {
				ptp_verbose_log("error reading data packet\n");
				return PTP_IO_ERR;
			}
			read += rc;
		}

		// Read response packet
		int resp_len = read - (int)c->length;
		c = (struct PtpBulkContainer *)(r->data + c->length);
		if (resp_len >= 4) {
			int rest = c->length - resp_len;
			if (rest != 0) {
				rc = ptp_cmd_read(r, r->data + read, rest);
				if (rc < 0) {
					ptp_verbose_log("error reading resposne packet\n");
					return PTP_IO_ERR;
				}
				read += rc;
			}

			if (resp_len > c->length) {
				ptp_verbose_log("Read too much of packet\n");
				return PTP_IO_ERR;
			}
		} else if (read - c->length >= 32) {
			// packet read
		} else {
			// Read response packet
			rc = ptp_cmd_read(r, r->data + read, r->max_packet_size);
			if (rc == 0) {
				CAMLIB_SLEEP(100);
				rc = ptp_cmd_read(r, r->data + read, r->max_packet_size);
			}
			if (rc < 0) {
				printf("Error reading response packet 2\n");
				return PTP_IO_ERR;
			}
			read += rc;
		}

	}

	return 0;
}

// For USB packets over IP, we can't do any LibUSB/LibWPD performance tricks. reads will just time out.
int ptpipusb_read_packet(struct PtpRuntime *r, int of) {
	int rc = 0;
	int read = 0;

	while (r->wait_for_response) {
		rc = ptpip_cmd_read(r, r->data + of + read, 4);

		r->wait_for_response--;

		if (rc > 0) break;

		if (r->wait_for_response) {
			ptp_verbose_log("Trying again...\n");
			CAMLIB_SLEEP(CAMLIB_WAIT_MS);
		}
	}

	r->wait_for_response = r->response_wait_default;

	if (rc < 0) {
		ptp_verbose_log("Failed to read packet length: %d\n", rc);
		return PTP_COMMAND_IGNORED;
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

	// Update struct after resize
	h = (struct PtpBulkContainer *)(r->data + of);

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
		if (rc == PTP_COMMAND_IGNORED) rc = PTP_IO_ERR; // Command was not ignored as we got a data packet
		if (rc < 0) return rc;

		read += rc;
	}

	ptp_verbose_log("ptpipusb_receive_bulk_packets: Read %d bytes\n", read);
	ptp_verbose_log("ptpipusb_receive_bulk_packets: Return code: 0x%X\n", ptp_get_return_code(r));

	return read;
}

int ptp_receive_all_packets(struct PtpRuntime *r) {
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
