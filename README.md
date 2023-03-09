# camlib
This is a portable PTP/USB/IP library written in C99. This uses no code from gphoto2, libptp, or libmtp.  
This is a complete rewrite from the ground up, and corrects the mistakes made in the design of older libraries.  

This library is written primarily for my [CamControl](https://camcontrol.danielc.dev/) Android app.

## Design
- Data parsing, packet building, and packet sending/recieving is all done in a single buffer
- Core library will perform no memory allocation
- No platform specific code at the core
- Modular design - easy to pick and pull out parts
- No macros, unless it's for debugging - everything is a function that can be accessed from other languages

## Checklist
- [x] Working Linux, Android, and Windows backends
- [x] Bindings to other languages
- [x] Real time camera previews (EOS)
- [x] Complete most EOS/Canon vendor OCs
- [x] Take pictures, perform bulb, set properties, get properties, poll events
- [x] Basic filesystem functionality
- [x] Finish basic Canon functions
- [ ] Dummy reciever (a virtual camera for test communication, fuzz testing)
- [ ] Basic Nikon, Sony, Fuji support

## Sample
```
struct PtpRuntime r;
ptp_generic_init(&r);

if (ptp_device_init(&r)) {
	puts("Device connection error");
	return 0;
}

struct PtpDeviceInfo di;

ptp_get_device_info(&r, &di);
ptp_device_info_json(&di, (char*)r.data, r.data_length);
printf("%s\n", (char*)r.data);

ptp_device_close(&r);
ptp_generic_close(&r);
```
