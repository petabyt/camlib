// PicLib specific code, not PTP ISO standard definitions
#ifndef PTP_LIB_H
#define PTP_LIB_H

#include <stdint.h>

struct PtpRuntime {
	int transaction;
    uint8_t *data;
    int data_length;
};

// Generic command structure - order of data isn't important.
// Meant to be a bridge to the other packet types.
struct PtpCommand {
	int code;

	int params[5];
	int param_length;

	uint8_t *data;
	int data_length;
};

// Helper functions
uint16_t ptp_read_uint16(void **dat);
uint16_t ptp_read_uint32(void **dat);
void ptp_read_string(void **dat, char *string, int max);
int ptp_read_uint16_array(void **dat, uint16_t *buf, int max);

// packet builders
int ptp_recv_packet(struct PtpRuntime *r, uint16_t code, uint32_t params[5], int param_length, int read_size);
int ptp_recv_packet_pre(struct PtpRuntime *r, uint16_t code);

// To store unpacked device info data
struct PtpDeviceInfo {
	uint16_t standard_version;
	uint32_t vendor_ext_id;
	uint16_t version;
	char extensions[128];
	uint16_t functional_mode;

	int ops_supported_length;
	uint16_t ops_supported[256];

	int events_supported_length;
	uint16_t events_supported[128];

	int props_supported_length;
	uint16_t props_supported[64];

	int capture_formats_length;
	uint16_t capture_formats[16];

	int playback_formats_length;
	uint16_t playback_formats[16];

	char manufacturer[128];
	char model[128];
	char device_version[64];
	char serial_number[128];
};

#endif
