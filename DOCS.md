## Quick Overview of PTP Standard
1. "command" packet
2. "data" packet
3. recieve data back

### int ptp_recv_packet(struct PtpRuntime *r, uint16_t code, uint32_t params[5], int param_length, int read_size)
```
int packet_size = ptp_recv_packet(&r, 0x1001, (uint16_t[]){}, 0, 0x0);
```
