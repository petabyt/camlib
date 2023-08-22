// Common IO backend code - only applies to platforms that have generic
// USB packet IO access (Not Windows)

// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>

int ptp_send_bulk_packets(struct PtpRuntime *r, int length) {
	ptp_verbose_log("send_bulk_packets 0x%X (%s)\n", ptp_get_return_code(r), ptp_get_enum_all(ptp_get_return_code(r)));

	int sent = 0;
	int x;
	while (1) {
		if (r->connection_type == PTP_USB) {
			x = ptp_send_bulk_packet(r->data + sent, length);
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

int ptp_recieve_bulk_packets(struct PtpRuntime *r) {
	if (r->connection_type == PTP_IP) {
		return ptpip_recieve_bulk_packets(r);
	} else {
		return ptpusb_recieve_bulk_packets(r);
	}
}

int ptpip_recieve_bulk_packets(struct PtpRuntime *r) {
	int read = 0;

	int rc = ptpip_cmd_read(r, r->data, 8);
	if (rc != 8) {
		ptp_verbose_log("Failed to read PTP/IP header\n");

		// If specified, retry indefinitely
		for (int i = 0; i < r->wait_for_response; i++) {
			rc = ptpip_cmd_read(r, r->data + read, 8);
			if (rc == 8) {
				goto finally_read;
			}
		}

		return PTP_IO_ERR;
	} else {
		finally_read:;
		read += rc;
	}

	r->wait_for_response = 0;

	// Check for socket kill signal (?)
	uint32_t *test = (uint32_t *)r->data;
	if (test[0] == 0x8 && test[1] == 0xffffffff) {
		ptp_verbose_log("Recieved kill sig\n");
		return PTP_IO_ERR;
	}

	// Read in remaining data

	struct PtpIpResponseContainer *c = (struct PtpIpBulkContainer *)(r->data);

	// if (type == PTPIP_DATA_PACKET_END || type == 0) {
		// return ptpip_recieve_bulk_packets(r);
	// }

	if (c->length == 0) {
		ptp_verbose_log("Recieved unfinished packet %X %X %X", c->length, c->type, c->code);
		return PTP_IO_ERR;
	}

	while (read != c->length) {
		rc = ptpip_cmd_read(r, r->data + read, c->length - read);
		if (rc < 0) return PTP_IO_ERR;

		read += rc;

		if (read > c->length) {
			ptp_verbose_log("Catastrophic error - read too many bytes\n");
			return PTP_IO_ERR;
		}
	}

	// Read in response packet
	if (c->type == PTP_PACKET_TYPE_DATA) {
		c = (struct PtpIpResponseContainer *)(r->data + read);
		rc = ptpip_cmd_read(r, r->data + read, 4);
		if (rc != 4) return PTP_IO_ERR;
		read += rc;
		rc = ptpip_cmd_read(r, r->data + read, c->length - 4);
		if (rc < 0) return PTP_IO_ERR;
		read += rc;
	}

	ptp_verbose_log("recieve_bulk_packets: Read %d bytes\n", read);
	ptp_verbose_log("recieve_bulk_packets: Return code: 0x%X\n", ptp_get_return_code(r));

	return read;
}

int ptpusb_recieve_bulk_packets(struct PtpRuntime *r) {
	int read = 0;
	int x;
	while (1) {
		if (r->connection_type == PTP_USB) {
			x = ptp_recieve_bulk_packet(r->data + read, r->max_packet_size);
		} else if (r->connection_type == PTP_IP || r->connection_type == PTP_IP_USB) {
			x = ptpip_cmd_read(r, r->data + read, r->max_packet_size);

			if (x > 0 && read == 0) {
				// If we recieve an event on the wrong pipe, assume error
				uint32_t *test = (uint32_t *)r->data;
				if (test[0] == PTPIP_EVENT) {
					// Shutdown event
					if (test[1] == 0xffffffff) {
						ptp_verbose_log("Recieved shutdown event");
						return PTP_IO_ERR;
					}

					// TODO: Might be possible to skip events
					return PTP_IO_ERR;
				}
			}
		}

		if (x < 0) {
			// Check if first time reading, try again once
			if (read == 0) {
				ptp_verbose_log("Failed to recieve packet, trying again...\n");
				CAMLIB_SLEEP(100);
				if (r->connection_type == PTP_USB) {
					x = ptp_recieve_bulk_packet(r->data + read, r->max_packet_size);
				} else if (r->connection_type == PTP_IP || r->connection_type == PTP_IP_USB) {
					x = ptpip_cmd_read(r, r->data + read, r->max_packet_size);
				}
			}

			if (x < 0) {
				ptp_verbose_log("recieve_bulk_packet: %d\n", x);
				return PTP_IO_ERR;
			}
		}
		read += x;

		if (read >= r->data_length - r->max_packet_size) {
			ptp_verbose_log("recieve_bulk_packets: Not enough memory\n");
			return PTP_OUT_OF_MEM;
		}

		if (x != r->max_packet_size) {
			ptp_verbose_log("recieve_bulk_packets: Read %d bytes\n", read);
			struct PtpBulkContainer *c = (struct PtpBulkContainer *)(r->data);

			// Read the response packet if only a data packet was sent (may be larger than 0xc bytes sometimes)
			if (c->type == PTP_PACKET_TYPE_DATA) {
				if (r->connection_type == PTP_USB) {
					x = ptp_recieve_bulk_packet(r->data + read, r->max_packet_size);
				} else if (r->connection_type == PTP_IP || r->connection_type == PTP_IP_USB) {
					x = ptpip_cmd_read(r, r->data + read, r->max_packet_size);
				}

				ptp_verbose_log("recieve_bulk_packets: Recieved extra packet %d bytes\n", x);
			}

			ptp_verbose_log("recieve_bulk_packets: Return code: 0x%X\n", ptp_get_return_code(r));

			return read;
		}
	}
}


int ptp_fsend_packets(struct PtpRuntime *r, int length, FILE *stream) {
	//ptp_verbose_log("send_bulk_packets 0x%X\n", ptp_get_return_code(r));

	int x = ptp_send_bulk_packet(r->data, length);
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
			x = ptp_send_bulk_packet(r->data, x);
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
int ptp_frecieve_bulk_packets(struct PtpRuntime *r, FILE *stream, int of) {
	int read = 0;

	// Since the data is written to file, we must remember the packet type
	int type = -1;
	while (1) {
		int x = ptp_recieve_bulk_packet(r->data, r->max_packet_size);
		if (x < 0) {
			ptp_verbose_log("recieve_bulk_packet: %d\n", x);
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
			ptp_verbose_log("recieve_bulk_packets: Read %d bytes\n", read);

			// Read the response packet if only a data packet was sent
			if (type == PTP_PACKET_TYPE_DATA) {
				x = ptp_recieve_bulk_packet(r->data, r->max_packet_size);
				ptp_verbose_log("recieve_bulk_packets: Return code: 0x%X\n", ptp_get_return_code(r));
			} else {
				// TODO: Why send a small packet with stream reader?
			}

			return read;
		}
	}
}
