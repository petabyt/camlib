// Test directprint things
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpict.h>

int main() {
	struct PtpRuntime r;
	ptp_init(&r);

	struct PtpDeviceInfo di;

	if (ptp_device_init(&r)) {
		puts("Device connection error");
		return 0;
	}

	ptp_open_session(&r);

	struct PtpArray *arr;
	int rc = ptp_get_storage_ids(&r, &arr);
	if (arr->length == 0) abort();
	int id = arr->data[0];

	struct PtpObjectInfo oi;
	memset(&oi, 0, sizeof(oi));
	oi.obj_format = 0x3002;
	strcpy(oi.filename, "HDISCVRY.DPS");

	ptp_send_object_info(&r, id, 0, &oi);

	ptp_close_session(&r);
	ptp_device_close(&r);
	ptp_close(&r);

	return 0;
}

