#include <stdio.h>
#include <string.h>

#include "camlib.h"

int ptp_parse_object_info(struct PtpRuntime *r, struct PtpObjectInfo *oi) {
	void *d = ptp_get_payload(r);
	memcpy(d, oi, PTP_OBJ_INFO_VAR_START);
	d += PTP_OBJ_INFO_VAR_START;
	ptp_read_string(&d, oi->filename, sizeof(oi->filename));
	ptp_read_string(&d, oi->date_created, sizeof(oi->date_created));
	ptp_read_string(&d, oi->date_modified, sizeof(oi->date_modified));
	ptp_read_string(&d, oi->keywords, sizeof(oi->keywords));

	return 0;
}

int ptp_pack_object_info(struct PtpRuntime *r, struct PtpObjectInfo *oi) {
	void *d = ptp_get_payload(r);
	memcpy(oi, d, PTP_OBJ_INFO_VAR_START);
	d += PTP_OBJ_INFO_VAR_START;

	ptp_write_string(&d, oi->filename);
	ptp_write_string(&d, oi->date_created);
	ptp_write_string(&d, oi->date_modified);
	ptp_write_string(&d, oi->keywords);

	return 0;
}
