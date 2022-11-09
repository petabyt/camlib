#include <stdlib.h>
#include <stdio.h>
#include <jni.h>
#include <string.h>

#include <ptp.h>
#include <camlib.h>
#include <operations.h>

int ptp_liveview_size() {
    return (720 * 480 * 4);
}

int ptp_liveview_frame(struct PtpRuntime *r, uint8_t *buffer) {
    int a = ptp_custom_recieve(r, 0x9997);
    if (a < 0) {
        return a;
    } else if (a == 12) {
        return -1;
    }

    uint8_t *data = r->data + 12;
    int length = (360 * 240);

    for (int i = 0; i < length; i++) {
        buffer[0] = data[0];
        buffer[1] = data[1];
        buffer[2] = data[2];
        buffer[3] = 0x0;
        buffer += 4;
        data += 3;
    }

    return 0;
}
