// Scan filesystem
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>

int main() {
	struct PtpRuntime r;
	ptp_generic_init(&r);

	struct PtpDeviceInfo di;

	if (ptp_device_init(&r)) {
		puts("Device connection error");
		return 1;
	}

	ptp_open_session(&r);

	struct UintArray *arr;
	int rc = ptp_get_storage_ids(&r, &arr);
	if (rc) return rc;
	int id = arr->data[0];

	rc = ptp_get_object_handles(&r, id, 0, 0, &arr);
	if (rc) return rc;
	arr = ptp_dup_uint_array(arr);

	for (int i = 0; i < arr->length; i++) {
		struct PtpObjectInfo oi;
		rc = ptp_get_object_info(&r, arr->data[i], &oi);
		if (rc) return 1;

		printf("Filename: %s\n", oi.filename);
		printf("File size: %u\n", oi.compressed_size);
	}

	free(arr);

	char buffer[PTP_BIND_DEFAULT_SIZE];
	rc = bind_run(&r, "ptp_init", buffer, sizeof(buffer));
	printf("ptp_init: %s\n", buffer);

	rc = bind_run(&r, "ptp_connect", buffer, sizeof(buffer));
	printf("ptp_connect: %s\n", buffer);

	rc = bind_run(&r, "ptp_open_session", buffer, sizeof(buffer));
	printf("ptp_open_session: %s\n", buffer);

	rc = bind_run(&r, "ptp_get_device_info", buffer, sizeof(buffer));
	printf("ptp_get_device_info: %s\n", buffer);

	rc = bind_run(&r, "ptp_close_session", buffer, sizeof(buffer));
	printf("ptp_close_session: %s\n", buffer);

	rc = bind_run(&r, "ptp_disconnect", buffer, sizeof(buffer));
	printf("ptp_disconnect: %s\n", buffer);

	//ptp_device_close(&r);
	ptp_generic_close(&r);
	return 0;
}
