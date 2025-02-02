#error
#include <stdio.h>
#include <stdint.h>
#include <pcap.h>

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


void packet_handler(u_char *user_data, const struct pcap_pkthdr *pkthdr, const u_char *packet) {
	struct usbpcap_header *header = (struct usbpcap_header *)packet;

	FILE *f = (FILE *)user_data;

	fwrite(packet + header->header_len, 1, pkthdr->len - header->header_len, f);

	printf("Packet length: %d\n", pkthdr->len - header->header_len);
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: %s <pcap_file>\n", argv[0]);
		return 1;
	}

	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t *handle = pcap_open_offline(argv[1], errbuf);

	if (handle == NULL) {
		fprintf(stderr, "Error opening pcap file: %s\n", errbuf);
		return 1;
	}

	FILE *f = fopen("DUMP", "wb");
	if (f == NULL) return -1;

	if (pcap_loop(handle, 0, packet_handler, (void *)f) < 0) {
		fprintf(stderr, "Error reading packets: %s\n", pcap_geterr(handle));
		return 1;
	}

	fclose(f);

	pcap_close(handle);
	return 0;
}
