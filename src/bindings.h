#ifndef BINDINGS_H
#define BINDINGS_H

#define PTP_TIMEOUT 1000

int ptp_device_init();
int ptp_send_bulk_packet();
int ptp_recieve_bulk_packets();
int ptp_device_close();

#endif
