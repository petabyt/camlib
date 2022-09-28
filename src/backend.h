#ifndef BINDINGS_H
#define BINDINGS_H

#define PTP_TIMEOUT 1000

#include "camlib.h"

int ptp_device_init(struct PtpRuntime *r);
int ptp_send_bulk_packets(struct PtpRuntime *r, int length);
int ptp_recieve_bulk_packets(struct PtpRuntime *r);
int ptp_device_close(struct PtpRuntime *r);

#endif
