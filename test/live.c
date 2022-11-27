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
#include "../../rawdraw/rawdraw_sf.h"
#include "../../rawdraw/os_generic.h"
int running = 1;
void HandleKey( int keycode, int bDown ) {
	if (keycode == CNFG_KEY_ESCAPE) running = 0;
}
void HandleButton( int x, int y, int button, int bDown ) { }
void HandleMotion( int x, int y, int mask ) { }
void HandleDestroy() { }

uint32_t rgb(int r, int g, int b) {
	uint32_t c = 0;
	uint8_t *x = (uint8_t*)&c;
	x[0] = b;
	x[1] = g;
	x[2] = r;
	x[3] = 0;

	return c;
}

#define WIDTH 720 / 2
#define HEIGHT 480 / 2


// precompute some parts of YUV to RGB computations
int yuv2rgb_RV[256];
int yuv2rgb_GU[256];
int yuv2rgb_GV[256];
int yuv2rgb_BU[256];

/** http://www.martinreddy.net/gfx/faqs/colorconv.faq
 * BT 601:
 * R'= Y' + 0.000*U' + 1.403*V'
 * G'= Y' - 0.344*U' - 0.714*V'
 * B'= Y' + 1.773*U' + 0.000*V'
 * 
 * BT 709:
 * R'= Y' + 0.0000*Cb + 1.5701*Cr
 * G'= Y' - 0.1870*Cb - 0.4664*Cr
 * B'= Y' - 1.8556*Cb + 0.0000*Cr
 */

void precompute_yuv2rgb()
{
    /*
    *R = *Y + ((1437 * V) >> 10);
    *G = *Y -  ((352 * U) >> 10) - ((731 * V) >> 10);
    *B = *Y + ((1812 * U) >> 10);
    */
    for (int u = 0; u < 256; u++)
    {
        int8_t U = u;
        yuv2rgb_GU[u] = (-352 * U) >> 10;
        yuv2rgb_BU[u] = (1812 * U) >> 10;
    }

    for (int v = 0; v < 256; v++)
    {
        int8_t V = v;
        yuv2rgb_RV[v] = (1437 * V) >> 10;
        yuv2rgb_GV[v] = (-731 * V) >> 10;
    }
}

#define COERCE(x,lo,hi) MAX(MIN((x),(hi)),(lo))

#define MIN(a,b) \
   ({ typeof ((a)+(b)) _a = (a); \
      typeof ((a)+(b)) _b = (b); \
     _a < _b ? _a : _b; })

#define MAX(a,b) \
   ({ typeof ((a)+(b)) _a = (a); \
       typeof ((a)+(b)) _b = (b); \
     _a > _b ? _a : _b; })

void yuv2rgb(int Y, int U, int V, int* R, int* G, int* B)
{
    const int v_and_ff = V & 0xFF;
    const int u_and_ff = U & 0xFF;
    int v = Y + yuv2rgb_RV[v_and_ff];
    *R = COERCE(v, 0, 255);
    v = Y + yuv2rgb_GU[u_and_ff] + yuv2rgb_GV[v_and_ff];
    *G = COERCE(v, 0, 255);
    v = Y + yuv2rgb_BU[u_and_ff];
    *B = COERCE(v, 0, 255);
}

void ml_live() {
	precompute_yuv2rgb();
	ptp_open_session(&r);

	CNFGSetup( "Magic Lantern Live view", WIDTH, HEIGHT );

	uint32_t *frame = malloc(WIDTH * HEIGHT * 4);
	int frames = 0;
	while (CNFGHandleInput() && running) {
		puts("Waiting...");
		int v = ptp_custom_recieve(&r, 0x9997);
		puts("Recieved");
		printf("Size: %d\n", v);
	
		CNFGClearFrame();

		uint8_t *data = r.data + 12;

		int x = 0;
		for (int i = 0; i < WIDTH * HEIGHT * 3; i += 3) {
			//int r, g, b;
			//yuv2rgb(data[i], data[i + 1], data[i + 2], &r, &g, &b);
			r = data[i]; g = data[i + 1]; b = data[i + 2];
			frame[x] = rgb(r, g, b);
			x++;
		}

		CNFGBlitImage(frame, 0, 0, WIDTH, HEIGHT);

		char txtBuf[64];
		sprintf(txtBuf, "Frames: %d", frames);
		CNFGColor(0xffffffff);
		CNFGPenX = 1; CNFGPenY = 1;
		CNFGDrawText( txtBuf, 2 );

		//Display the image and wait for time to display next frame.
		CNFGSwapBuffers();
		//OGUSleep( (int)( 1000000 ) );
		frames++;
	}


	free(frame);
	ptp_close_session(&r);
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

