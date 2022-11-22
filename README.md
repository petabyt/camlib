# camlib
This is a portable PTP/USB/IP library written in C99. This uses no code from gphoto2, libptp, or libmtp.  
This is a complete rewrite from the ground up, and corrects the mistakes made in the design of older libraries.  

## Design
- Data parsing, packet building, and packet sending/recieving is all done in a single buffer
- Core library will perform no memory allocation
- No platform specific code at the core
- Try and make it small, and easy to pick and pull out parts
- No macros, unless it's for debugging - everything is a function that can be accessed from other languages

## Checklist
- [x] Generate a valid packet
- [x] Android USB Backend
- [x] Basic PTP definition header
- [x] Stable back and forth communication (Java/android, LibUSB)
- [x] Parse device info
- [x] Bindings to other languages or USB/IP backends
- [x] Real time camera previews
- [x] Complete most EOS/Canon vendor OCs
- [ ] Transfer files, delete files
- [ ] "Frontend" functions - generic wrappers for vendor specific things

## Compiling
Compile test binary for Windows:
```
make WIN=1 wintest.exe
```
Compile test binary for Linux:
```
make optest
```
