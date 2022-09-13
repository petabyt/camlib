# piclib
Meant to be a portable PTP library. This is a complete rewrite and uses no code from [libptp2](https://github.com/leirf/libptp).  
Functions in this lib will either respond to platform specific bindings, or return raw packet data. 

## Checklist
- [x] Generate a valid packet
- [x] Android USB Backend
- [x] Basic PTP definition header
- [x] Stable back and forth communication (android backend)
- [x] Parse device info
- [ ] Bindings to other languages or USB/IP backends
- [ ] Real time camera previews

## Design
- Data parsing, packet building, and packet sending/recieving is all done in a single buffer
- No memory allocation by core lib
- No platform specific code at the core
- C99
- Try and make it small, and easy to pick and pull out parts

## License
### For commercial software
If you want to use this library (as well as the Java USB/IP bindings) for commercial software,  
contact me (brikbusters@gmail.com) for a quote on a commercial license.
### For free/open-source software
You may fork, use, modify, distribute, and freely use this library under the GNU GPL v2.0 or Later.  
