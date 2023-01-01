# PTP decoder
- At top level, run `make dec`
- `./dec ~/Downloads/canon_usb_dump foo.txt`

Input file can be any binary dump format, it doesn't matter what is between the packets.
As long as the packet structure is intact, the decoder will find it through brute-force.
