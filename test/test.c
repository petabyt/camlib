// Unit testing for camlib
// Designed for vcam libusb spoofer
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include <camlib.h>

int test_setup_usb(struct PtpRuntime *r) {
	ptp_init(r);

	if (ptp_device_init(r)) {
		puts("Device connection error");
		return 1;
	}

	int rc = ptp_open_session(r);
	if (rc) return rc;	

	assert(ptp_get_return_code(r) == PTP_RC_OK);

	return 0;
}

int ptp_vcam_magic() {
	struct PtpRuntime r;

	int rc = test_setup_usb(&r);
	if (rc) return rc;

	int sizes[] = {4, 10, 101, 513, 1000, 997, 257, 511, 1, 2, 3};
	size_t l = sizeof(sizes) / sizeof(int);

	// Try to send a bunch of different payloads and make sure the data is recieved correctly
	for (size_t i = 0; i < l; i++) {
		struct PtpCommand cmd;
		cmd.code = 0xBEEF;
		cmd.param_length = 1;

		srand(time(NULL));

		unsigned char *buffer = malloc(sizes[i]);

		// Random data, basic checksum
		int checksum = 0;
		for (int x = 0; x < sizes[i]; x++) {
			buffer[x] = rand() / 256;
			checksum += buffer[x];
		}

		cmd.params[0] = checksum;

		rc = ptp_send_data(&r, &cmd, buffer, sizes[i]);
		if (rc) return rc;
	}

	rc = ptp_eos_evproc_run(&r, "EnableBootDisk %d 'aasd'", 10);

	ptp_close(&r);
	return 0;
}

// Test case for EOS T6/1300D vcam
int test_eos_t6() {
	struct PtpRuntime r;

	struct PtpDeviceInfo di;

	int rc = test_setup_usb(&r);
	if (rc) return rc;

	rc = ptp_get_device_info(&r, &di);
	if (rc) return rc;
	char buffer[4096];
	ptp_device_info_json(&di, buffer, sizeof(buffer));
	printf("%s\n", buffer);

	if (ptp_device_type(&r) == PTP_DEV_EOS) {
		ptp_eos_set_remote_mode(&r, 1);
		ptp_eos_set_event_mode(&r, 1);
	}

	rc = ptp_eos_get_event(&r);
	if (rc) return rc;

	char events_json[4096];
	ptp_eos_events_json(&r, events_json, sizeof(events_json));

	printf("Events: %s\n", events_json);

	assert(!strcmp(di.manufacturer, "Canon Inc."));
	assert(!strcmp(di.extensions, "G-V: 1.0;"));
	assert(!strcmp(di.model, "Canon EOS Rebel T6"));

	struct PtpArray *arr;
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
	ptp_close(&r);
	return 0;
}

int test_props() {
	struct PtpRuntime r;

	int rc = test_setup_usb(&r);
	if (rc) return rc;

	rc = ptp_get_prop_value(&r, PTP_PC_BatteryLevel);
	if (rc) return rc;
	printf("Test parse: %d\n", ptp_parse_prop_value(&r));
	assert(ptp_parse_prop_value(&r) == 50);

	struct PtpPropDesc pd;
	rc = ptp_get_prop_desc(&r, PTP_PC_BatteryLevel, &pd);
	if (rc) return rc;
	assert(pd.current_value32 == 50);
	assert(pd.default_value32 == 50);

	assert(pd.range_form.min == 0);
	assert(pd.range_form.max == 100);
	assert(pd.range_form.step == 1);

	rc = ptp_get_prop_desc(&r, PTP_PC_ImageSize, &pd);
	if (rc) return rc;

	char buffer[128];
	ptp_read_string(pd.default_value, buffer, sizeof(buffer));
	assert(!strcmp(buffer, "640x480"));

	ptp_close(&r);
	return 0;
}

int test_fs() {
	struct PtpRuntime r;

	int rc = test_setup_usb(&r);
	if (rc) return rc;

	struct PtpDeviceInfo di;
	rc = ptp_get_device_info(&r, &di);
	if (rc) return rc;
	char buffer[4096];
	ptp_device_info_json(&di, buffer, sizeof(buffer));
	printf("%s\n", buffer);

	struct PtpArray *arr;
	rc = ptp_get_storage_ids(&r, &arr);
	if (rc) return rc;
	int id = arr->data[0];

	assert(arr->length == 1); // vcam only has 1 fs

	struct PtpStorageInfo si;
	rc = ptp_get_storage_info(&r, arr->data[0], &si);
	if (rc) return rc;

	ptp_storage_info_json(&si, buffer, sizeof(buffer));
	printf("%s\n", buffer);

/* Not implemented in vcam!
	struct PtpObjectInfo oi = {0};
	strcpy(oi.filename, "test.jpg");
	rc = ptp_send_object_info(&r, id, 0, &oi);
	if (rc) return rc;
*/

	rc = ptp_get_object_handles(&r, id, 0, 0, &arr);
	if (rc) return rc;
	assert(ptp_get_return_code(&r) == PTP_RC_OK);

	for (int i = 0; i < arr->length; i++) {
		struct PtpObjectInfo oi;
		rc = ptp_get_object_info(&r, arr->data[i], &oi);
		if (rc) return rc;

		printf("Filename: %s\n", oi.filename);
		printf("File size: %u\n", oi.compressed_size);
	}

	struct PtpObjectInfo oi;
	rc = ptp_get_object_info(&r, 0xdeadbeef, &oi);
	if (rc) return rc;
	assert(!strcmp(oi.filename, "test.png"));
	assert(oi.compressed_size == 1234);

	free(arr);

	ptp_close(&r);
	return 0;
}

static void *thread(void *arg) {
	struct PtpRuntime *r = (struct PtpRuntime *)arg;

	if ((rand() & 1) == 0) {
		struct PtpDeviceInfo di;
		int rc = ptp_get_device_info(r, &di);
		if (rc) goto err;
		char buffer[4096];
		ptp_device_info_json(&di, buffer, sizeof(buffer));
		printf("%s\n", buffer);
	} else {
		struct PtpArray *arr;
		int rc = ptp_get_storage_ids(r, &arr);
		if (rc) goto err;
		printf("%X\n", arr->data[0]);
		if (arr->data[0] != 0x10001) goto err;
	}

	pthread_exit(0);
	err:;
	printf("Error in thread %d\n", getpid());
	ptp_close(r);
	exit(1);
}

static int test_multithread() {
	struct PtpRuntime r;

	int rc = test_setup_usb(&r);
	if (rc) return rc;

	for (int i = 0; i < 100; i++) {
		pthread_t th;
		if (pthread_create(&th, NULL, thread, (void *)&r) != 0) {
			perror("pthread_create() error");
			exit(1);
		}
	}

	usleep(100000);

	//ptp_device_close(&r);

	return 0;	
}

int main() {
	int rc;

	rc = test_multithread();
	printf("Return code: %d\n", rc);
	if (rc) return rc;

	rc = test_eos_t6();
	printf("Return code: %d\n", rc);
	if (rc) return rc;

	rc = test_fs();
	printf("Return code: %d\n", rc);
	if (rc) return rc;

	rc = ptp_vcam_magic();
	printf("Return code: %d\n", rc);
	if (rc) return rc;

	rc = test_props();
	printf("Return code: %d\n", rc);
	if (rc) return rc;

	return 0;
}
