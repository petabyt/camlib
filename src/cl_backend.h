#ifndef CL_BACKEND_H
#define CL_BACKEND_H

#define PTP_TIMEOUT 1000

// Linked-list entry for a single USB device
struct PtpDeviceEntry {
	struct PtpDeviceEntry *prev;

	int id;
	int vendor_id;
	int product_id;

	uint32_t endpoint_in;
	uint32_t endpoint_out;
	uint32_t endpoint_int;

	char name[16];
	char manufacturer[16];
	void *device_handle_ptr;

	struct PtpDeviceEntry *next;
};

DLL_EXPORT int ptp_comm_init(struct PtpRuntime *r);
DLL_EXPORT struct PtpDeviceEntry *ptpusb_device_list(struct PtpRuntime *r);
DLL_EXPORT void ptpusb_free_device_list(struct PtpDeviceEntry *e);
DLL_EXPORT int ptp_device_open(struct PtpRuntime *r, struct PtpDeviceEntry *entry);

// Init comm (if not already) and connect to the first device available
DLL_EXPORT int ptp_device_init(struct PtpRuntime *r);

// Temporary :)
#define ptp_send_bulk_packet DEPRECATED_USE_ptp_cmd_write_INSTEAD
#define ptp_receive_bulk_packet DEPRECATED_USE_ptp_cmd_read_INSTEAD

// Bare IO, send a single 512 byte packet. Return negative or NULL on error.
DLL_EXPORT int ptp_cmd_write(struct PtpRuntime *r, void *to, int length);
DLL_EXPORT int ptp_cmd_read(struct PtpRuntime *r, void *to, int length);

// Reset the pipe, can clear issues
DLL_EXPORT int ptp_device_reset(struct PtpRuntime *r);

// Recieve all packets, and whatever else (common logic for all backends)
DLL_EXPORT int ptp_send_bulk_packets(struct PtpRuntime *r, int length);
DLL_EXPORT int ptp_receive_bulk_packets(struct PtpRuntime *r);
DLL_EXPORT int ptp_read_int(struct PtpRuntime *r, void *to, int length);

DLL_EXPORT int ptp_device_close(struct PtpRuntime *r); // TODO: Disconnect, confusing with ptp_close

// Upload file data as packets, but upload r->data till length first
DLL_EXPORT int ptp_fsend_packets(struct PtpRuntime *r, int length, FILE *stream);

// Reads the incoming packet to file, starting after an optional offset
DLL_EXPORT int ptp_freceive_bulk_packets(struct PtpRuntime *r, FILE *stream, int of);

DLL_EXPORT int ptpip_connect(struct PtpRuntime *r, const char *addr, int port);
DLL_EXPORT int ptpip_cmd_write(struct PtpRuntime *r, void *data, int size);
DLL_EXPORT int ptpip_cmd_read(struct PtpRuntime *r, void *data, int size);

DLL_EXPORT int ptpip_connect_events(struct PtpRuntime *r, const char *addr, int port);
DLL_EXPORT int ptpip_event_send(struct PtpRuntime *r, void *data, int size);
DLL_EXPORT int ptpip_event_read(struct PtpRuntime *r, void *data, int size);

DLL_EXPORT int ptpip_close(struct PtpRuntime *r); // TODO: Disconnect, confusing with ptp_close

#endif
