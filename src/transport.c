// Packet transport interface for PTPUSB, PTPIP, and PTPUSBIP - uses OS IO functions
// For Windows MTP, this calls functions in libwpd.c
// Copyright 2024 by Daniel C (https://github.com/petabyt/camlib)

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>

int ptp_send_packet(struct PtpRuntime *r, int length) {
	int sent = 0;
	while (1) {
		int max = length - sent;
		if (r->max_packet_size != 512) ptp_panic("max packet size is not 512, %d", r->max_packet_size);
		if (max > r->max_packet_size) max = r->max_packet_size;
		int rc = ptp_cmd_write(r, r->data + sent, max);

		if (rc < 0) {
			ptp_verbose_log("%s: %d\n", __func__, rc);
			return PTP_IO_ERR;
		}
		
		sent += rc;
		
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

		int pk2_of = pk1_of + (int)h->length;

		rc = ptpip_read_packet(r, pk2_of);
		if (rc < 0) return rc;
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

int ptpusb_read_all_packets(struct PtpRuntime *r) {
	r->wait_for_response = r->response_wait_default;

	int read = 0;
	while (1) {
		int rc;
		if (r->connection_type == PTP_USB) {
			rc = ptp_cmd_read(r, r->data + read, r->max_packet_size);
		} else if (r->connection_type == PTP_IP_USB) {
			rc = ptpip_cmd_read(r, r->data + read, r->max_packet_size);
		} else {
			ptp_panic("illegal connection type");
		}
		if (rc < 0 && r->wait_for_response) {
			ptp_verbose_log("Response error %d, trying again", rc);
			r->wait_for_response--;
			continue;
		} else if (rc < 0) {
			return PTP_IO_ERR;
		}
		read += rc;

		// TODO: if read is 0 for long enough, return PTP_COMMAND_IGNORED

		// Min packet size is at least read
		if (read < 12) continue;

		uint32_t length;
		uint16_t type;
		struct PtpBulkContainer *c = (struct PtpBulkContainer *)(r->data);
		ptp_read_u32(&c->length, &length);
		ptp_read_u16(&c->type, &type);

		// First packet is at least read
		if (read < length) continue;

		if (type == PTP_PACKET_TYPE_RESPONSE) {
			break;
		} else if (type == PTP_PACKET_TYPE_DATA) {
			// Min data packet is at least read
			if (length + 12 > read) continue;

			c = (struct PtpBulkContainer *)(r->data + length);
			uint32_t data_length;
			ptp_read_u32(&c->length, &data_length);
			ptp_read_u16(&c->type, &type);
			if (type != PTP_PACKET_TYPE_RESPONSE) {
				ptp_verbose_log("Expected response packet but got %d\n", type);
				return PTP_IO_ERR;
			}
			if (read == data_length + length) {
				break;
			} else if (read > data_length + length) {
				ptp_verbose_log("Read too much data %d\n", read);
				return PTP_IO_ERR;
			}
		}
	}

	return 0;
}

int ptp_receive_all_packets(struct PtpRuntime *r) {
	if (r->connection_type == PTP_IP) {
		return ptpip_receive_bulk_packets(r);
	} else if (r->connection_type == PTP_USB || r->connection_type == PTP_IP_USB) {
		return ptpusb_read_all_packets(r);
	} else {
		ptp_panic("Unknown connection type %d", r->connection_type);
	}
}
