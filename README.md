# camlib
This is a portable Picture Transfer Protocol (PTP) library written from scratch in C.

[Documentation](https://danielc.dev/camlib/)

## Design
- Data parsing, packet building, and packet sending/recieving is all done in a single buffer (grows as needed)
- Will not perform memory allocations between operations
- Portable, works well on many different platforms
- No macros, only clean C API - everything is a function that can be accessed from other languages
- I'm writing this while writing [vcam](git@github.com:petabyt/vcam.git) at the same time,
so my vendor opcode implementations are (maybe) more reliabile than others ;)

## Checklist
- [x] Complete working implemention of PTP as per ISO 15740
- [x] Working Linux, Android, and Windows backends
- [x] JSON bindings for high level languages
- [x] Real time camera previews (EOS, Magic Lantern)
- [x] Implement most EOS/Canon vendor OCs
- [x] ISO PTP/IP implementation
- [x] ~~Fuji WiFi and USB support~~ (code moved to https://github.com/petabyt/fudge/)
- [x] Lua bindings (for embedding)
- [x] (Mostly) thread safe
- [x] Regression testing (vcam)
- [ ] Sony support
- [ ] Pentax support

## Sample
Get device info:
```
#include <camlib.h>

int main() {
	struct PtpRuntime *r = ptp_new(PTP_USB);

	if (ptp_device_init(r)) {
		printf("Device connection error\n");
		return 0;
	}

	struct PtpDeviceInfo di;

	char buffer[2048];
	ptp_get_device_info(r, &di);
	ptp_device_info_json(&di, buffer, sizeof(buffer));
	printf("%s\n", buffer);

	ptp_device_close(r);
	ptp_close(r);
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
return ptp_send(r, &cmd);

// Send a command with data payload
struct PtpCommand cmd;
cmd.code = 0x1234;
cmd.param_length = 1;
cmd.params[0] = 1234;
uint32_t dat[2] = {123, 123};
return ptp_send_data(r, &cmd, dat, sizeof(dat));
```
Explore the filesystem:
```
struct UintArray *arr;
int rc = ptp_get_storage_ids(r, &arr);
int id = arr->data[0];

rc = ptp_get_object_handles(r, id, PTP_OF_JPEG, 0, &arr);

for (int i = 0; i < arr->length; i++) {
	struct PtpObjectInfo oi;
	ptp_get_object_info(r, arr->data[i], &oi);
	printf("Filename: %s\n", oi.filename);
}

free(arr);
```

## License
Camlib is licensed under the Apache License 2.0.  
lua-cjson: http://tools.ietf.org/html/rfc4627 (MIT License)  
