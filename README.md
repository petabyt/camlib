# piclib
Meant to be a portable PTP library. This is a complete rewrite and uses no code from [libptp2](https://github.com/leirf/libptp).

Functions in this lib will either respond to platform specific bindings, or just return packet data.

## Checklist
- [x] Generate a valid packet
- [x] Android USB Backend
- [x] Basic PTP definition header
- [x] Stable back and forth communication (android backend)
- [ ] Bindings to other languages or USB/IP backends
- [ ] Real time camera previews
