#ifndef BINDINGS_H
#define BINDINGS_H

#define PTP_TIMEOUT 1000

#include <camlib.h>

// Connect to the first device available
int ptp_device_init(struct PtpRuntime *r);

// Bare IO, send a single 512 byte packet. Return negative or NULL on error.
int ptp_send_bulk_packet(void *to, int length);
int ptp_recieve_bulk_packet(void *to, int length);

// Reset the pipe, can clear issues
int ptp_device_reset(struct PtpRuntime *r);

// Recieve all packets, and whatever else (common logic for all backends)
int ptp_send_bulk_packets(struct PtpRuntime *r, int length);
int ptp_recieve_bulk_packets(struct PtpRuntime *r);
int ptp_recieve_int(void *to, int length);

int ptp_device_close(struct PtpRuntime *r);

// Upload file data as packets, but upload r->data till length first
int ptp_fsend_packets(struct PtpRuntime *r, int length, FILE *stream);

// Reads the incoming packet to file, starting after an optional offset
int ptp_frecieve_bulk_packets(struct PtpRuntime *r, FILE *stream, int of);

int ptpip_connect(struct PtpRuntime *r, char *addr, int port);
int ptpip_send_bulk_packet(struct PtpRuntime *r, void *data, int sizeBytes);
int ptpip_recieve_bulk_packet(struct PtpRuntime *r, void *data, int sizeBytes);
int ptpip_close(struct PtpRuntime *r);

#endif
