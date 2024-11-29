/** \file */ 
#ifndef CL_BACKEND_H
#define CL_BACKEND_H

/// @defgroup Backend Backend interface
/// @brief Layer over OS-dependent I/O interface
/// @addtogroup Backend
/// @{

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

/// @brief Get a linked list of USB or PTP Devices
/// @returns linked list of devices or NULL if no devices are connected (or OS error)
struct PtpDeviceEntry *ptpusb_device_list(struct PtpRuntime *r);

void ptpusb_free_device_list(struct PtpDeviceEntry *e);

/// @brief Open and connect to a device from the PtpDeviceEntry structure
/// @note Sets kill switch to 0
int ptp_device_open(struct PtpRuntime *r, struct PtpDeviceEntry *entry);

/// @brief Connects to the first PTP device it finds
int ptp_device_init(struct PtpRuntime *r);

/// @brief Send data over the raw command endpoint
int ptp_cmd_write(struct PtpRuntime *r, void *to, int length);
/// @brief Receive raw data over the command endpoint
int ptp_cmd_read(struct PtpRuntime *r, void *to, int length);

/// @brief Reset the USB endpoints if possible
int ptp_device_reset(struct PtpRuntime *r);

/// @brief Send packets in r->data
int ptp_send_packet(struct PtpRuntime *r, int length);

/// @brief Receive all packets into r->data
int ptp_receive_all_packets(struct PtpRuntime *r);

/// @brief Poll the interrupt endpoint
int ptp_read_int(struct PtpRuntime *r, void *to, int length);

/// @brief Disconnect from the current device
int ptp_device_close(struct PtpRuntime *r);

/// @brief Connect to a TCP port on the default network adapter
/// @note Sets kill switch to 0
int ptpip_connect(struct PtpRuntime *r, const char *addr, int port, int extra_tmout);
int ptpip_cmd_write(struct PtpRuntime *r, void *data, int size);
int ptpip_cmd_read(struct PtpRuntime *r, void *data, int size);

int ptpip_connect_events(struct PtpRuntime *r, const char *addr, int port);
int ptpip_event_send(struct PtpRuntime *r, void *data, int size);
int ptpip_event_read(struct PtpRuntime *r, void *data, int size);

int ptpip_close(struct PtpRuntime *r);

void ptpusb_free_device_list_entry(void *);

/// @brief Get status of connected device
int ptpusb_get_status(struct PtpRuntime *r);

/// @}

#endif
