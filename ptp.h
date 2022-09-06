// This file is for defining standard PTP stuff
#ifndef PTP_H
#define PTP_H

#include <stdint.h>

struct PtpContainer {
	uint16_t code;
	uint32_t session;
	uint32_t transaction;
	uint32_t param1;
	uint32_t param2;
	uint32_t param3;
	uint32_t param4;
	uint32_t param5;
	uint8_t nparam;
};

struct PtpBulkContainer {
	uint32_t length;
	uint16_t type;
	uint16_t code;
	uint32_t transaction;

	uint32_t param1;
	uint32_t param2;
	uint32_t param3;
	uint32_t param4;
	uint32_t param5;

	// Payload data follows
};

struct PtpEventContainer {
	uint32_t length;
	uint16_t type;
	uint16_t code;
	uint32_t transaction;
	uint32_t param1;
	uint32_t param2;
	uint32_t param3;
};

// Standard PTP OCs
#define PTP_OC_GetDeviceInfo 0x1001
#define PTP_OC_OpenSession 0x1002
#define PTP_OC_CloseSession 0x1003
#define PTP_OC_GetStorageIDs 0x1004
#define PTP_OC_GetNumObjects 0x1006
#define PTP_OC_GetObjectHandles 0x1007
#define PTP_OC_GetObjectInfo 0x1008
#define PTP_OC_GetObject 0x1009
#define PTP_OC_GetThumb 0x100A
#define PTP_OC_DeleteObject 0x100B

// Return codes
#define PTP_RC_Undefined 0x2000
#define PTP_RC_OK 0x2001

// Event Codes
#define PTP_EV_Undefined 0x4000
#define PTP_EV_CancelTransaction 0x4001

// Vendor init/USB codes
#define VENDOR_CANON 1193

// PTP Packet types
#define PACKET_TYPE_COMMAND 1
#define PACKET_TYPE_DATA 2
#define PACKET_TYPE_RESPONSE 3
#define PACKET_TYPE_EVENT 4

#endif
