// Scan filesystem
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <libpict.h>

int main() {
	struct PtpRuntime *r = ptp_new(0);

	struct PtpDeviceInfo di;

	if (ptp_device_init(r)) {
		puts("Device connection error");
		goto cleanup;
	}

	ptp_open_session(r);

	struct PtpArray *arr;
	int rc = ptp_get_storage_ids(r, &arr);
	if (arr->length == 0) {
		puts("No storage devices found.");
		goto exit;
	}
	int id = arr->data[0];

	rc = ptp_get_object_handles(r, id, PTP_OF_JPEG, 0, &arr);

	for (int i = 0; i < arr->length; i++) {
		struct PtpObjectInfo oi;
		ptp_get_object_info(r, arr->data[i], &oi);

		printf("Filename: %s\n", oi.filename);
		printf("File size: %u\n", oi.compressed_size);
	}

	free(arr);

	exit:;
	ptp_close_session(r);
	ptp_device_close(r);
	cleanup:;
	ptp_close(r);
	return 0;
}

