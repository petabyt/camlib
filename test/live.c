#include <stdio.h>
#include <stdio.h>
#include <stdint.h>

#include <camlib.h>

#include "../../tigr/tigr.h"

#define SCREEN_WIDTH 720
#define SCREEN_HEIGHT 480

int main() {
	Tigr* screen = tigrWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello", 0);

	struct PtpRuntime r;
	ptp_generic_init(&r);

	struct PtpDeviceInfo di;

	if (ptp_device_init(&r)) {
		puts("Device connection error");
		return 0;
	}

	ptp_open_session(&r);

	while (!tigrClosed(screen) && !tigrKeyDown(screen, TK_ESCAPE)) {
		int rc = ptp_ml_init_bmp_lv(&r);

		uint32_t *buffer = NULL;
		rc = ptp_ml_get_bmp_lv(&r, &buffer);
		if (rc) break;
		if (buffer == NULL) break;

		for (int i = 0; i < screen->w * screen->h; i++) {
			screen->pix[i].r = 0xff;
			screen->pix[i].g = 0;
			screen->pix[i].b = 0;
			screen->pix[i].a = 0;
		}

		for (int i = 0; i < screen->w * screen->h; i++) {
			uint8_t A = (buffer[i] >> 24) & 0xFF;
			if (A == 0xFF) {
				continue;
			}

			screen->pix[i].r = (buffer[i] >> 16) & 0xFF;
			screen->pix[i].g = (buffer[i] >> 8) & 0xFF;
			screen->pix[i].b = (buffer[i]) & 0xFF;
			screen->pix[i].a = 0;
		}

		free(buffer);

        tigrUpdate(screen);
    }
    tigrFree(screen);

}
