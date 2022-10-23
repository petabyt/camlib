#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>
#include <backend.h>
#include <ptp.h>
#include <operations.h>

struct PtpRuntime r;

void print_bytes(uint8_t *bytes, int n) {
	for (int i = 0; i < n; i++) {
		if (bytes[i] > 31 && bytes[i] < 128) {
			printf("'%c' ", bytes[i]);
		} else {
			printf("%02X ", bytes[i]);
		}
	}

	puts("");
}

#define SIZE 5000000

void info() {
	struct PtpDeviceInfo di;

	if (ptp_device_init(&r)) {
		puts("Device init error");
		return;
	}

	ptp_get_device_info(&r, &di);

	printf("Return code: 0x%X\n", ptp_get_return_code(&r));

	ptp_device_info_json(&di, (char*)r.data, r.data_length);
	printf("%s\n", (char*)r.data);

	ptp_device_close(&r);
}

#if 1
#define CNFG_IMPLEMENTATION
#include "../rawdraw/rawdraw_sf.h"
#include "../rawdraw/os_generic.h"
void HandleKey( int keycode, int bDown ) { }
void HandleButton( int x, int y, int button, int bDown ) { }
void HandleMotion( int x, int y, int mask ) { }
void HandleDestroy() { }

uint32_t rgb(int r, int g, int b) {
	uint32_t c = 0;
	uint8_t *x = (uint8_t*)&c;
	x[0] = b;
	x[1] = g;
	x[2] = r;
	x[3] = r;

	return c;
}

void ml_live() {
	ptp_open_session(&r);

	CNFGSetup( "Magic Lantern Live view", 720, 480 );

	while(CNFGHandleInput())
	{

			puts("Waiting...");
	int v = ptp_custom_recieve(&r, 0x9997);
	puts("Recieved");
	printf("Size: %d\n", v);
	printf("Return code: 0x%X\n", ptp_get_return_code(&r));
	
		CNFGClearFrame();

		int i = 12;
		for (int y = 0; y < 480; y++) {
			for (int x = 0; x < 720; x++) {
				CNFGColor(rgb(r.data[i], r.data[i + 1], r.data[i + 2]));
				CNFGTackPixel(x, y);
				i += 3;
			}
		}

		//Display the image and wait for time to display next frame.
		CNFGSwapBuffers();		
		//OGUSleep( (int)( 0.5 * 1000 ) );
	}

}

#endif

int main() {
	r.data = malloc(SIZE);
	r.transaction = 0;
	r.session = 0;
	r.data_length = SIZE;

	struct PtpDeviceInfo di;

	if (ptp_device_init(&r)) {
		puts("Device connection error");
		return 0;
	}


	ptp_open_session(&r);

	ml_live();

	ptp_device_close(&r);

	return 0;
}

