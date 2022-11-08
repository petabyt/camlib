#ifndef PTPENUM_H
#define PTPENUM_H

int ptp_enum(char *string);
char *ptp_get_enum(int id);

enum PtpType {
	PTP_OC,
	PTP_OF,
	PTP_PC,
	PTP_EC,
	PTP_RC,
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
