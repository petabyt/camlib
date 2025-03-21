#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <libpict.h>
#include <string.h>
#include <assert.h>
#include <pcap.h>

enum PacketType {
	ETHERNET,
	USBPCAP,
};

struct Context {
	int vendor;
	int last_transaction;
	int unfinished_packet;
	int transaction;
	int last_type;
	FILE *f;
	FILE *raw_out;
	enum PacketType packet_type;
};

int decode_eos_evproc(FILE *f, int length, uint8_t *data) {
	int of = 0;
	char buffer[64];

	of += ptp_read_utf8_string(data + of, buffer, sizeof(buffer));

	uint32_t len, temp, type, val, p3, p4;
	of += ptp_read_u32(data + of, &len);

	fprintf(f, "- Command name: %s\n", buffer);
	fprintf(f, "- Parameters: %u\n", len);

	for (int i = 0; i < (int)len; i++) {
		of += ptp_read_u32(data + of, &type);
		if (type == 2) {
			of += ptp_read_u32(data + of, &val);
			of += ptp_read_u32(data + of, &p3);
			of += ptp_read_u32(data + of, &p4);
			fprintf(f, "- Int param: %X (%X, %X)\n", val, p3, p4);
			of += ptp_read_u32(data + of, &temp); // size
		} else if (type == 4) {
			of += ptp_read_u32(data + of, &val);
			of += ptp_read_u32(data + of, &temp);
			of += ptp_read_u32(data + of, &temp);
			of += ptp_read_u32(data + of, &temp);
			of += ptp_read_u32(data + of, &temp);
			of += ptp_read_u32(data + of, &temp);

			of += ptp_read_utf8_string(data + of, buffer, sizeof(buffer));
			fprintf(f, "- String param: %s\n", buffer);
		} else {
			fprintf(f, "Unknown type %x\n", type);
		}
	}

	return 0;
}

int ptp_dump_packet(struct Context *ctx, unsigned long int file_of, const uint8_t *bytes, unsigned int length) {
	if (ctx->unfinished_packet) {
		fprintf(ctx->f, "split packet %d/%d ...\n", length, ctx->unfinished_packet);
		if (length > ctx->unfinished_packet) {
			// Detect a packet sneaking in after long transfers
			ctx->unfinished_packet = 0;
			bytes += ctx->unfinished_packet;
			length -= ctx->unfinished_packet;
		} else {
			ctx->unfinished_packet -= length;
			return 0;
		}
	}

	int type = PTP_OC;
	struct PtpBulkContainer *c = (struct PtpBulkContainer *)bytes;

	char *newline = "\n";
	if (ctx->transaction == -1) newline = "";

	if (c->type == PTP_PACKET_TYPE_COMMAND) {
		if (c->length > sizeof(struct PtpBulkContainer) || c->length < 12) {
			return -1;
		}

		if (c->code == PTP_OC_OpenSession) {
			ctx->transaction = 0;
		}

		fprintf(ctx->f, "%s--- COMMAND Container --- \n", newline);
	} else if (c->type == PTP_PACKET_TYPE_DATA) {
		if (ctx->last_type == PTP_PACKET_TYPE_COMMAND) {
			fprintf(ctx->f, "--- DATA Container ---\n");
		} else if (ctx->last_type == PTP_PACKET_TYPE_DATA) {
			fprintf(ctx->f, "%s--- Random DATA Container\n", newline);
		}
	} else if (c->type == PTP_PACKET_TYPE_RESPONSE) {
		fprintf(ctx->f, "%s--- RESPONSE Container ---\n", newline);
		type = PTP_RC;
	}

	char *enm = ptp_get_enum(type, ctx->vendor, c->code);
	if (enm == enum_null) {
		enm = "??";
	}

	fprintf(ctx->f, "Offset:\t0x%lX\n", file_of);
	fprintf(ctx->f, "Length:\t(%u)\n", c->length);
	fprintf(ctx->f, "Transaction ID:\t(%u)\n", c->transaction);
	fprintf(ctx->f, "Code:\t(0x%X) %s\n", c->code, enm);

	if (c->type == PTP_PACKET_TYPE_COMMAND) {
		fprintf(ctx->f, "Params:\t");
		int pl = (c->length - 12) / 4;
		for (int i = 0; i < pl; i++) {
			fprintf(ctx->f, "%X ", c->params[i]);
		}
		if (pl == 0) {
			fprintf(ctx->f, "No params");
		}
		fprintf(ctx->f, "\n");
	} else if (c->type == PTP_PACKET_TYPE_DATA) {
		fprintf(ctx->f, "Data:\t");
		int pl = c->length - 12;

		if (length < c->length) {
			ctx->unfinished_packet = c->length - length;
		}

		if (ctx->raw_out) {
			fwrite(bytes + 12, 1, pl, ctx->raw_out);
		}

		for (int i = 0; i < pl; i++) {
			fprintf(ctx->f, "%02X", bytes[i + 12]);
			if (i >= 50) {
				fprintf(ctx->f, "...");
				break;
			}
		}
		if (pl == 0) {
			fprintf(ctx->f, "No data");
		}
		fprintf(ctx->f, "\n");
#if 0
		if (c->code == 0x9052) {
			decode_eos_evproc(f, pl, bytes + addr + 12);
		}
#endif

		ctx->last_transaction = (int)c->transaction;
	}

	ctx->last_type = c->type;

	if (ctx->transaction != -1) {
		ctx->transaction++;
	}

	return 0;
}

struct __attribute__((packed)) usbpcap_header {
	uint16_t header_len;
	uint64_t irp_id;
	uint32_t status;
	uint32_t function;
	uint32_t info;
	uint16_t bus;
	uint16_t device;
	uint16_t endpoint;
	uint16_t transfer_type;
	uint64_t timestamp;
	uint32_t data_length;
};

struct __attribute__((packed)) ethernet_header {
	uint8_t dest[6];
	uint8_t src[6];
};

void packet_handler(u_char *user_data, const struct pcap_pkthdr *pkthdr, const u_char *packet) {
	static int of = 0;
	struct Context *ctx = (struct Context *)user_data;

	int header_len;
	if (ctx->packet_type == USBPCAP) {
		header_len = 0x36;
	} else if (ctx->packet_type == ETHERNET) {
		const struct usbpcap_header *header = (const struct usbpcap_header *)packet;
		header_len = (int)header->header_len;
	} else { abort(); }

	unsigned int data_len = (pkthdr->len - header_len);
	if (data_len == 0) {
		// no data packet, probably ACK or something
		return;
	}

	//printf("%d\n", data_len);

	ptp_dump_packet(ctx, of, (const uint8_t *)packet + header_len, data_len);

	of += pkthdr->caplen;
}


static int decode_pcap(struct Context *ctx, const char *input) {
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t *handle = pcap_open_offline(input, errbuf);

	if (handle == NULL) {
		fprintf(stderr, "Error opening pcap file: %s\n", errbuf);
		return 1;
	}

	if (pcap_loop(handle, 0, packet_handler, (void *)ctx) < 0) {
		fprintf(stderr, "Error reading packets: %s\n", pcap_geterr(handle));
		return 1;
	}

	pcap_close(handle);

	return 0;
}

int ptp_decode_output(const char *mode, const char *input, const char *output) {
	FILE *fi = fopen(input, "rb");
	if (fi == NULL) {
		puts("error opening file");
		return 1;
	}

	fseek(fi, 0, SEEK_END);
	long size = ftell(fi);
	fseek(fi, 0, SEEK_SET);
	uint8_t *buffer = malloc(size + 100);
	assert(buffer);
	fread(buffer, 1, size, fi);
	fclose(fi);

	struct Context ctx = {0};
	ctx.f = fopen(output, "w");
	if (ctx.f == NULL) {
		puts("error creating file");
		return 1;
	}
	if (!strcmp(mode, "wifi")) {
		ctx.packet_type = USBPCAP;
	} else if (!strcmp(mode, "usb")) {
		ctx.packet_type = ETHERNET;
	} else { abort(); }
	ctx.unfinished_packet = 0;
	ctx.transaction = -1; // -1 is unknown transaction
	ctx.last_transaction = 0;
	ctx.last_type = PTP_PACKET_TYPE_RESPONSE;
	ctx.vendor = PTP_DEV_EOS;

	fprintf(ctx.f, "Generated by libpict PTP Decoder\n");
	fprintf(ctx.f, "Parsing file %s\n", input);

	ctx.raw_out = fopen("DUMP", "wb");

	// Dumb way of checking if input file is pcap
	if (((uint32_t *)buffer)[0] == 0xa1b2c3d4) {
		free(buffer);
		decode_pcap(&ctx, input);
	} else {
		long addr = 0;
		while (addr < size) {
			const struct PtpBulkContainer *c = (const struct PtpBulkContainer *)(buffer + addr);

			int vendor_oc = c->code >> 8;

			int filter = 1;
			filter &= c->type == PTP_PACKET_TYPE_COMMAND || c->type == PTP_PACKET_TYPE_DATA
				|| c->type == PTP_PACKET_TYPE_RESPONSE || c->type == PTP_PACKET_TYPE_EVENT;

			if (c->type == PTP_PACKET_TYPE_COMMAND || c->type == PTP_PACKET_TYPE_DATA) {
				filter &= vendor_oc == 0x10 || vendor_oc == 0x90 || vendor_oc == 0x91 || vendor_oc == 0x92 || vendor_oc == 0x98 || vendor_oc == 0xF0;
			} else if (c->type == PTP_PACKET_TYPE_RESPONSE) {
				filter &= vendor_oc == 0x20;
			}

			//if (transaction != -1 && (last_transaction - 10) > c->transaction) goto cnt;

			if (filter) {
				int rc = ptp_dump_packet(&ctx, addr, buffer + addr, c->length);
				if (rc) return rc;
			}

			addr++;
		}

		free(buffer);
	}

	if (ctx.raw_out) {
		fclose(ctx.raw_out);
	}
	fclose(ctx.f);
	printf("Wrote to %s\n", output);
	return 0;
}
