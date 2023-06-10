# camlib
This is a portable PTP/USB library written in C99. This isn't a fork of gphoto2, libptp, or libmtp.  
This is a complete rewrite from the ground up, and is written to be maintainable and platform independent.  

This library is currently being used in [mlinstall](https://github.com/petabyt/mlinstall), [CamControl](https://camcontrol.danielc.dev/), and [EOS Intervalometer](https://play.google.com/store/apps/details?id=dev.petabyt.caminterv).

## Design
- Data parsing, packet building, and packet sending/recieving is all done in a single buffer
- Core library will perform almost no memory allocation, to avoid memory leaks
- No platform specific code at the core
- No macros, only clean C API - everything is a function that can be accessed from other languages

## Checklist
- [x] Complete working implemention of PTP as per ISO 15740
- [x] Working Linux, Android, and Windows backends
- [x] JSON bindings for high level languages
- [x] Real time camera previews (EOS, ML)
- [x] Complete most EOS/Canon vendor OCs
- [x] PTP/IP WiFi Implementation
- [x] Fuji WiFi support
- [ ] Sony support

## Sample
Get device info:
```
#include <camlib.h>

int main() {
	struct PtpRuntime r;
	ptp_generic_init(&r);

	if (ptp_device_init(&r)) {
		puts("Device connection error");
		return 0;
	}

	struct PtpDeviceInfo di;

	char buffer[2048];
	ptp_get_device_info(&r, &di);
	ptp_device_info_json(&di, buffer, sizeof(buffer));
	printf("%s\n", buffer);

	ptp_device_close(&r);
	ptp_generic_close(&r);
	return 0;
}
```
Calling a custom opcode:
```
// Send a command, and recieve packet(s)
struct PtpCommand cmd;
cmd.code = 0x1234;
cmd.param_length = 3;
cmd.params[0] = 420;
cmd.params[1] = 420;
cmd.params[2] = 420;
return ptp_generic_send(r, &cmd);

// Send a command with data payload
struct PtpCommand cmd;
cmd.code = 0x1234;
cmd.param_length = 1;
cmd.params[0] = 1234;
uint32_t dat[2] = {123, 123};
return ptp_generic_send_data(r, &cmd, dat, sizeof(dat));
```
Explore the filesystem:
```
struct UintArray *arr;
int rc = ptp_get_storage_ids(&r, &arr);
int id = arr->data[0];

rc = ptp_get_object_handles(&r, id, PTP_OF_JPEG, 0, &arr);
arr = ptp_dup_uint_array(arr);

for (int i = 0; i < arr->length; i++) {
	struct PtpObjectInfo oi;
	ptp_get_object_info(&r, arr->data[i], &oi);
	printf("Filename: %s\n", oi.filename);
}

free(arr);
```
