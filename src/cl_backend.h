#ifndef CL_BACKEND_H
#define CL_BACKEND_H

#define PTP_TIMEOUT 1000

/// @brief Linked-list entry for a single USB device
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

/// @brief Initialize communications context for the current thread
int ptp_comm_init(struct PtpRuntime *r);

/// @brief Get a linked list of USB or PTP Devices
/// @memberof PTP/USB
struct PtpDeviceEntry *ptpusb_device_list(struct PtpRuntime *r);
/// @memberof PTP/USB
void ptpusb_free_device_list(struct PtpDeviceEntry *e);
/// @brief Open and connect to a device from the PtpDeviceEntry structure
/// @memberof PTP/USB
int ptp_device_open(struct PtpRuntime *r, struct PtpDeviceEntry *entry);

/// @brief Runs ptp_comm_init and connect to the first device available
int ptp_device_init(struct PtpRuntime *r);

// Temporary :)
//#define ptp_send_bulk_packet DEPRECATED_USE_ptp_cmd_write_INSTEAD
//#define ptp_receive_bulk_packet DEPRECATED_USE_ptp_cmd_read_INSTEAD

/// @brief Send data over the raw command endpoint
/// @memberof PTP/USB
int ptp_cmd_write(struct PtpRuntime *r, void *to, int length);
/// @brief Receive raw data over the command endpoint
/// @memberof PTP/USB
int ptp_cmd_read(struct PtpRuntime *r, void *to, int length);

/// @brief Reset the USB device or endpoint if there is communication issues
/// @memberof PTP/USB
int ptp_device_reset(struct PtpRuntime *r);

/// @brief Send packets in r->data
/// @memberof PTP/USB
int ptp_send_packet(struct PtpRuntime *r, int length);

/// @brief Receive all packets into r->data
/// @memberof PTP/USB
int ptp_receive_all_packets(struct PtpRuntime *r);

/// @brief Poll the interrupt endpoint
/// @memberof PTP/USB
int ptp_read_int(struct PtpRuntime *r, void *to, int length);

/// @brief Disconnect from the current device
/// @memberof PTP/USB
int ptp_device_close(struct PtpRuntime *r);

/// @brief Connect to a TCP port on the default network adapter
/// @memberof PTP/IP
int ptpip_connect(struct PtpRuntime *r, const char *addr, int port, int extra_tmout);
/// @memberof PTP/IP
int ptpip_cmd_write(struct PtpRuntime *r, void *data, int size);
/// @memberof PTP/IP
int ptpip_cmd_read(struct PtpRuntime *r, void *data, int size);

/// @memberof PTP/IP
int ptpip_connect_events(struct PtpRuntime *r, const char *addr, int port);
/// @memberof PTP/IP
int ptpip_event_send(struct PtpRuntime *r, void *data, int size);
/// @memberof PTP/IP
int ptpip_event_read(struct PtpRuntime *r, void *data, int size);

/// @memberof PTP/IP
int ptpip_close(struct PtpRuntime *r);

#endif
