// CamLib specific code, not PTP ISO standard definitions
#ifndef PTP_LIB_H
#define PTP_LIB_H

#include <stdint.h>

// Max timeout for response
#define PTP_TIMEOUT 1000

// Optional debug logging
#define VERBOSE
#ifdef VERBOSE
	#define PTPLOG(...) printf(__VA_ARGS__);
#else
	#define PTPLOG(...) /* */
#endif

// Generic IO error, not PTP return codes
enum PtpGeneralError {
	PTP_OK = 0,
	PTP_NO_DEVICE = -1,
	PTP_NO_PERM = -2,
	PTP_OPEN_FAIL = -3,
	PTP_OUT_OF_MEM = -4,
	PTP_IO_ERR = -5,
	PTP_PARSE_ERR = -6,
};

enum PtpConnType {
	PTP_IP,
	PTP_USB,
};

enum PtpDeviceType {
	PTP_DEV_EMPTY = 0,
	PTP_DEV_CANON,
	PTP_DEV_NIKON,
	PTP_DEV_SONY,
	PTP_DEV_FUJI,
	PTP_DEV_PANASONIC,
};

// Holds vital lib info - passed to most functions
// Initialize all to zero, then set data and data_length
struct PtpRuntime {
	int transaction; // transaction ID
	int session; // session ID
    uint8_t *data;
    int data_length;
	int max_packet_size;
	int active_connection;

	int device_type;
	struct PtpDeviceInfo *di;
};

// Generic command structure - not a packet
// Meant to be a bridge to the other packet types.
struct PtpCommand {
	int code;

	uint32_t params[5];
	int param_length;

	int data_length;
};

// Helper packet reader functions
uint16_t ptp_read_uint16(void **dat);
uint16_t ptp_read_uint32(void **dat);
void ptp_read_string(void **dat, char *string, int max);
int ptp_read_uint16_array(void **dat, uint16_t *buf, int max);
int ptp_read_uint32_array(void **dat, uint16_t *buf, int max);
int ptp_wide_string(char *buffer, int max, char *input);

// Helper packet writer functions
void ptp_write_uint8(void **dat, uint8_t b);
void ptp_write_string(void **dat, char *string);

// Packet builder functions
// A command packet is sent first (cmd), followed by a data packet
int ptp_new_data_packet(struct PtpRuntime *r, struct PtpCommand *cmd);
int ptp_new_cmd_packet(struct PtpRuntime *r, struct PtpCommand *cmd);

// Gets return code directly from runtime data
int ptp_get_return_code(struct PtpRuntime *r);

uint32_t ptp_get_param(struct PtpRuntime *r, int index);

int ptp_detect_device(struct PtpRuntime *r);
int ptp_check_opcode(struct PtpRuntime *r, int op);

// Get ptr of packet payload, after header
uint8_t *ptp_get_payload(struct PtpRuntime *r);

int ptp_generic_send(struct PtpRuntime *r, struct PtpCommand *cmd);

// Packets start with a uint32 representing the length of the packet.
// In some cases, we want to append more data to the packet, so the length
// value must be updated
void ptp_update_data_length(struct PtpRuntime *r, int length);

// Used only for open session
void ptp_update_transaction(struct PtpRuntime *r, int t);

#include "data.h"

int ptp_parse_device_info(struct PtpRuntime *r, struct PtpDeviceInfo *di);
int ptp_device_info_json(struct PtpDeviceInfo *di, char *buffer, int max);

int ptp_parse_object_info(struct PtpRuntime *r, struct PtpObjectInfo *oi);
int ptp_pack_object_info(struct PtpRuntime *r, struct PtpObjectInfo *oi);

// For emergencies
int ptp_dump(struct PtpRuntime *r);

#endif
