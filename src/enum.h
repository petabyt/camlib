#ifndef PTPENUM_H
#define PTPENUM_H

#define MAX_ENUM_LENGTH 64

int ptp_enum_all(char *string);
int ptp_enum(int type, char *string);
char *ptp_get_enum(int id);

enum PtpType {
	PTP_OC, // operation codes
	PTP_OF, // object formats
	PTP_PC, // property codes
	PTP_EC, // event codes
	PTP_RC, // return code
	PTP_ST, // storage type
	PTP_FT, // filesystem type
	PTP_AC, // access code
	PTP_ENUM, // regular enums
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
