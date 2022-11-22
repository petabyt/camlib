// "Polyfill" for missing MinGW support
#ifndef PTP_WIN_H
#define PTP_WIN_H

#define ESCAPE_PTP_VENDOR_COMMAND 0x100
#define ESCAPE_PTP_CLEAR_STALLS 0x200

#define PTP_MAX_PARAMS 5
#define PTP_NEXTPHASE_NO_DATA 5
#define PTP_NEXTPHASE_READ_DATA 3
#define PTP_NEXTPHASE_WRITE_DATA 4

// Pragmas prevent padding

#pragma pack(push, 1)
typedef struct _PTP_VENDOR_DATA_IN {
	uint16_t OpCode;
	uint32_t SessionId;
	uint32_t TransactionId;
	uint32_t Params[PTP_MAX_PARAMS];
	uint32_t NumParams;
	uint32_t NextPhase;
	uint8_t VendorWriteData[1];
} PTP_VENDOR_DATA_IN, *PPTP_VENDOR_DATA_IN;

typedef struct _PTP_VENDOR_DATA_OUT {
	uint16_t ResponseCode;
	uint32_t SessionId;
	uint32_t TransactionId;
	uint32_t Params[PTP_MAX_PARAMS];
	uint8_t VendorReadData[1];
}PTP_VENDOR_DATA_OUT, *PPTP_VENDOR_DATA_OUT;
#pragma pack(pop)

#endif
