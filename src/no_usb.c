// Include this if you wish to never use USB
#include <libpict.h>
#include <ptp.h>

int ptp_device_init(struct PtpRuntime *r) {
    return -1;
}

int ptp_device_close(struct PtpRuntime *r) {
    return -1;
}

int ptp_device_reset(struct PtpRuntime *r) {
    return -1;
}

int ptp_cmd_write(struct PtpRuntime *r, void *to, int length) {
    return -1;
}

int ptp_cmd_read(struct PtpRuntime *r, void *to, int length) {
    return -1;
}

int ptp_read_int(struct PtpRuntime *r, void *to, int length) {
    return -1;
}

int reset_int() {
    return -1;
}
