#ifndef PTPENUM_H
#define PTPENUM_H

int ptp_enum(char *string);
char *ptp_get_enum(int id);

enum PtpType {
	PTP_OC, // operation codes
	PTP_OF, // object formats
	PTP_PC, // property codes
	PTP_EC, // 
	PTP_RC, // return code
	PTP_ST, 
	PTP_FT,
	PTP_AC, // access code
	PTP_ENUM,
};

enum PtpVendor {
	PTP_VENDOR_CANON,
	PTP_VENDOR_GENERIC,
	PTP_VENDOR_NIKON,
	PTP_VENDOR_FUJI,
	PTP_VENDOR_SONY,
};

struct PtpEnum {
	int type;
	int vendor;
	char *name;
	int value;
};

extern int ptp_enums_length;
extern struct PtpEnum ptp_enums[];

#endif
