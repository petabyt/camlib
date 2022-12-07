// TODO: Rename to ptpenum.h?
#ifndef PTPENUM_H
#define PTPENUM_H

#include <camlib.h>

#define MAX_ENUM_LENGTH 64

int ptp_enum_all(char *string);
int ptp_enum(int type, char *string);
char *ptp_get_enum(int id);

extern char *enum_null;

enum PtpType {
	PTP_ENUM = 0, // regular enums
	PTP_OC = 1, // operation codes
	PTP_OF = 2, // object formats
	PTP_PC = 3, // property codes
	PTP_EC = 4, // event codes
	PTP_RC = 5, // return code
	PTP_ST = 6, // storage type
	PTP_FT = 7, // filesystem type
	PTP_AC = 8, // access code
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
