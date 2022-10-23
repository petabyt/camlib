#include <stdio.h>
#include <stdlib.h>

//Make it so we don't need to include any other C files in our build.
#define CNFG_IMPLEMENTATION

#include "../rawdraw/rawdraw_sf.h"

void HandleKey( int keycode, int bDown ) { }
void HandleButton( int x, int y, int button, int bDown ) { }
void HandleMotion( int x, int y, int mask ) { }
void HandleDestroy() { }

uint32_t rgb(int r, int g, int b) {
	uint32_t c = 0;
	uint8_t *x = (uint8_t*)&c;
	x[0] = 0xff;
	x[1] = g;
	x[2] = b;
	x[3] = r;

	return c;
}

int main()
{
	uint8_t *b = malloc(720 * 480 * 3);
	FILE *f = fopen("/home/daniel/Documents/sequoia-ptpy/test", "r");
	fread(b, 720 * 480 * 3, 1, f);
	fclose(f);

	CNFGSetup( "Example App", 1024, 768 );
	while(CNFGHandleInput())
	{
		CNFGClearFrame();

		int i = 0;
		for (int y = 0; y < 480; y++) {
			for (int x = 0; x < 720; x++) {
				CNFGColor(rgb(b[i], b[i+1], b[i+2]));
				CNFGTackPixel(x, y);
				i += 3;
			}
		}

		//Display the image and wait for time to display next frame.
		CNFGSwapBuffers();		
	}

	free(b);
}
