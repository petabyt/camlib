// Include this if you wish to never use USB
#include <camlib.h>
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

int ptp_cmd_write(void *to, int length) {
    return -1;
}

int ptp_cmd_read(void *to, int length) {
    return -1;
}

int ptp_read_int(void *to, int length) {
    return -1;
}

int reset_int() {
    return -1;
}
