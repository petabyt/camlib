# camlib
Meant to be a portable PTP library. This is a complete rewrite and uses no code from [libptp2](https://github.com/leirf/libptp).  
Functions in this lib will either respond to platform specific bindings, or return raw packet data. 

## Checklist
- [x] Generate a valid packet
- [x] Android USB Backend
- [x] Basic PTP definition header
- [x] Stable back and forth communication (Java/android, LibUSB)
- [x] Parse device info
- [x] Bindings to other languages or USB/IP backends
- [x] Real time camera previews
- [ ] Complete most EOS/Canon vendor OCs
- [ ] Transfer files, delete files
- [ ] "Frontend" functions - generic wrappers for vendor specific things

## Design
- Data parsing, packet building, and packet sending/recieving is all done in a single buffer
- No memory allocation by core lib
- No platform specific code at the core
- C99
- Try and make it small, and easy to pick and pull out parts
