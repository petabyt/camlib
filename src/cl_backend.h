#ifndef CL_BACKEND_H
#define CL_BACKEND_H

#define PTP_TIMEOUT 1000

// Connect to the first device available
int ptp_device_init(struct PtpRuntime *r);

// Bare IO, send a single 512 byte packet. Return negative or NULL on error.
int ptp_send_bulk_packet(void *to, int length);
int ptp_receive_bulk_packet(void *to, int length);

// Reset the pipe, can clear issues
int ptp_device_reset(struct PtpRuntime *r);

// Recieve all packets, and whatever else (common logic for all backends)
int ptp_send_bulk_packets(struct PtpRuntime *r, int length);
int ptp_receive_bulk_packets(struct PtpRuntime *r);
int ptp_receive_int(void *to, int length);

int ptp_device_close(struct PtpRuntime *r);

// Upload file data as packets, but upload r->data till length first
int ptp_fsend_packets(struct PtpRuntime *r, int length, FILE *stream);

// Reads the incoming packet to file, starting after an optional offset
int ptp_freceive_bulk_packets(struct PtpRuntime *r, FILE *stream, int of);

int ptpip_connect(struct PtpRuntime *r, char *addr, int port);
int ptpip_cmd_write(struct PtpRuntime *r, void *data, int size);
int ptpip_cmd_read(struct PtpRuntime *r, void *data, int size);

int ptpip_connect_events(struct PtpRuntime *r, char *addr, int port);
int ptpip_event_send(struct PtpRuntime *r, void *data, int size);
int ptpip_event_read(struct PtpRuntime *r, void *data, int size);

int ptpip_close(struct PtpRuntime *r);

#endif
