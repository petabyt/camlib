#ifndef PTPENUM_H
#define PTPENUM_H

enum PtpType {
	PTP_OC,
	PTP_OF,
	PTP_PC,
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

#endif
