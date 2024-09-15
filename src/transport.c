// Packet transport interface for PTPUSB, PTPIP, and PTPUSBIP - uses OS IO functions
// Don't include this file with Windows/LibWPD builds. LibWPD replaces this file.

// Copyright 2024 by Daniel C (https://github.com/petabyt/camlib)

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>

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
			ptp_verbose_log("send_bulk_packet: %d\n", __func__, x);
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

// Quirk of LibUSB/LibWPD - we can allowed read 512 bytes over and over again
// until we don't, then packet is over. This makes the code simpler and gives a reduces
// calls to the backend, which increases performance. This isn't possible with sockets - 
// the read will time out in most cases.
int ptpusb_read_all_packets(struct PtpRuntime *r) {
	int read = 0;

	// Try and get the first 512 bytes
	int rc = 0;
	while (rc <= 0 && r->wait_for_response) {
		rc = ptp_cmd_read(r, r->data + read, r->max_packet_size);

		r->wait_for_response--;

		if (rc < 0) break;

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

	// 512 is always enough for the response packet
	if (c->type == PTP_PACKET_TYPE_RESPONSE) {
		return 0;
	}

	// Reallocate enough to fill the rest of data and response packet
	if (c->type == PTP_PACKET_TYPE_DATA && r->data_length < (c->length + r->max_packet_size)) {
		rc = ptp_buffer_resize(r, c->length + r->max_packet_size);
		c = (struct PtpBulkContainer *)(r->data);
		if (rc) return rc;
	}

	while (read < c->length) {
		rc = ptp_cmd_read(r, r->data + read, r->max_packet_size);
		if (rc < 0) return PTP_IO_ERR;
		read += rc;
	}

	ptp_verbose_log("Read %d bytes\n", read);

	rc = ptp_cmd_read(r, r->data + read, r->max_packet_size);
	if (rc < 0) return PTP_IO_ERR;

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
	if (r->io_kill_switch) return -1;
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
