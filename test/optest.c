#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>
#include <backend.h>
#include <ptp.h>
#include <operations.h>
#define SIZE 300000
int main() {
	struct PtpRuntime r;

	memset(&r, 0, sizeof(struct PtpRuntime));
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

	ptp_eos_set_remote_mode(&r, 1);
	ptp_eos_set_event_mode(&r, 1);

	// while (1) {
	// ptp_eos_get_event(&r);
// 
	// char buffer[50000];
	// ptp_eos_events_json(&r, buffer, 50000);
	// puts(buffer);
		// sleep(1);
	// }

	char buf[1000];
	bind_run(&r, "ptp_set_property;\"image format\",1", buf, 1000);
	puts(buf);

	// uint32_t data[] = {
		// 0x1, 0x10, 0x1, 0xe, 0x2
	// };
// 
	// ptp_eos_set_prop_data(&r, 0xD120, data, sizeof(data));

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

	ptp_close_session(&r);
	ptp_device_close(&r);

	free(r.data);

	return 0;
}

