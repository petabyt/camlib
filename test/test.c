// Unit testing for camlib
// Designed for vcam libusb spoofer
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <camlib.h>

int test_setup_usb(struct PtpRuntime *r) {
	ptp_generic_init(r);

	if (ptp_device_init(r)) {
		puts("Device connection error");
		return 1;
	}

	int rc = ptp_open_session(r);
	if (rc) return rc;	

	assert(ptp_get_return_code(r) == PTP_RC_OK);

	return 0;
}

// Test case for EOS T6/1300D (most common DSLR)
int test_eos_t6() {
	struct PtpRuntime r;

	struct PtpDeviceInfo di;

	int rc = test_setup_usb(&r);
	if (rc) return rc;

	rc = ptp_get_device_info(&r, &di);
	if (rc) return rc;
	char buffer[2048];
	ptp_device_info_json(&di, buffer, sizeof(buffer));
	printf("%s\n", buffer);

	assert(!strcmp(di.manufacturer, "Canon Inc."));
	assert(!strcmp(di.extensions, "G-V: 1.0;"));
	assert(!strcmp(di.model, "Canon EOS Rebel T6"));

	struct UintArray *arr;
	rc = ptp_get_storage_ids(&r, &arr);
	if (rc) return rc;
	int id = arr->data[0];

	assert(arr->length == 1); // vcam only has 1 fs

	struct PtpStorageInfo si;
	rc = ptp_get_storage_info(&r, arr->data[0], &si);
	if (rc) return rc;

	ptp_storage_info_json(&si, buffer, sizeof(buffer));
	printf("%s\n", buffer);

	//ptp_device_close(&r);
	ptp_generic_close(&r);
	return 0;	
}

// TODO: Add JSON validator
int test_bind() {
	struct PtpRuntime r;

	int rc = test_setup_usb(&r);
	if (rc) return rc;

	char bbuffer[PTP_BIND_DEFAULT_SIZE];
	rc = bind_run(&r, "ptp_open_session", bbuffer, sizeof(bbuffer));
	printf("ptp_open_session: %s\n", bbuffer);

	rc = bind_run(&r, "ptp_get_device_info", bbuffer, sizeof(bbuffer));
	printf("ptp_get_device_info: %s\n", bbuffer);

	rc = bind_run(&r, "ptp_close_session", bbuffer, sizeof(bbuffer));
	printf("ptp_close_session: %s\n", bbuffer);

	rc = bind_run(&r, "ptp_disconnect", bbuffer, sizeof(bbuffer));
	printf("ptp_disconnect: %s\n", bbuffer);

	ptp_generic_close(&r);
	return 0;
}

int test_fs() {
	struct PtpRuntime r;

	int rc = test_setup_usb(&r);
	if (rc) return rc;

	struct PtpDeviceInfo di;
	rc = ptp_get_device_info(&r, &di);
	if (rc) return rc;
	char buffer[2048];
	ptp_device_info_json(&di, buffer, sizeof(buffer));
	printf("%s\n", buffer);

	struct UintArray *arr;
	rc = ptp_get_storage_ids(&r, &arr);
	if (rc) return rc;
	int id = arr->data[0];

	assert(arr->length == 1); // vcam only has 1 fs

	struct PtpStorageInfo si;
	rc = ptp_get_storage_info(&r, arr->data[0], &si);
	if (rc) return rc;

	assert(ptp_get_return_code(&r) == PTP_RC_OK);

	ptp_storage_info_json(&si, buffer, sizeof(buffer));
	printf("%s\n", buffer);

	rc = ptp_get_object_handles(&r, id, 0, 0, &arr);
	if (rc) return rc;
	assert(ptp_get_return_code(&r) == PTP_RC_OK);

	arr = ptp_dup_uint_array(arr);

	for (int i = 0; i < arr->length; i++) {
		struct PtpObjectInfo oi;
		rc = ptp_get_object_info(&r, arr->data[i], &oi);
		if (rc) return rc;

		printf("Filename: %s\n", oi.filename);
		printf("File size: %u\n", oi.compressed_size);
	}

	free(arr);

	ptp_generic_close(&r);
	return 0;
}

int main() {
	if (test_eos_t6()) return 1;
	if (test_bind()) return 1;
	if (test_fs()) return 1;

	return 0;
}
