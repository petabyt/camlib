#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>
#include <backend.h>
#include <ptp.h>
#include <operations.h>

void print_bytes(uint8_t *bytes, int n) {
	for (int i = 0; i < n; i++) {
		if (bytes[i] > 31 && bytes[i] < 128) {
			printf("'%c' ", bytes[i]);
		} else {
			printf("%02X ", bytes[i]);
		}
	}

	puts("");
}

#define SIZE 3000000

int main() {
	struct PtpRuntime r;

	memset(&r, sizeof(struct PtpRuntime), 0);
	r.data = malloc(SIZE);
	r.transaction = 0;
	r.session = 0;
	r.data_length = SIZE;

	struct PtpDeviceInfo di;

	if (ptp_device_init(&r)) {
		puts("Device connection error");
		return 0;
	}

	ptp_open_session(&r);

	ptp_get_device_info(&r, &di);
	ptp_device_info_json(&di, (char*)r.data, r.data_length);
	printf("%s\n", (char*)r.data);

	struct UintArray *arr;
	ptp_get_storage_ids(&r, &arr);
	int handle = arr->data[0];
	printf("Handle: %X\n", handle);

	struct PtpStorageInfo sinf;
	ptp_get_storage_info(&r, handle, &sinf);

	ptp_get_object_handles(&r, handle, 0, -1, &arr);
	handle = arr->data[0];
	printf("Obj handle %x\n", handle);

	struct PtpObjectInfo oi;
	ptp_get_object_info(&r, handle, &oi);
	printf("objinfo %s == %X\n", oi.filename, oi.compressed_size);

	ptp_eos_set_remote_mode(&r, 1);
	//ptp_eos_set_event_mode(&r, 1);


#if 0
	// Both produce 25/50/100
	struct PtpDevPropDesc pd = {0};
	ptp_get_prop_desc(&r, 0x5001, &pd);
	printf("Prop: %d\n", pd.current_value);
	ptp_dump(&r);

	ptp_get_prop_value(&r, 0x5001);
	void *d = ptp_get_payload(&r);
	printf("Prop2: %d\n", ptp_parse_data(&d, 2));
#endif

	//ptp_close_session(&r);
	ptp_device_close(&r);

	free(r.data);

	return 0;
}

