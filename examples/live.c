#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <camlib.h>

#include "../../tigr/tigr.h"

#define SCREEN_WIDTH 720
#define SCREEN_HEIGHT 480

int render_jpeg(int width, int height, TPixel *pix, void *input, int size) {
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	JSAMPARRAY buffer;
	int row_stride;

	// Initialize the JPEG decompression object
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	// Set the source file
	jpeg_mem_src(&cinfo, input, size);

	// Read the JPEG header
	jpeg_read_header(&cinfo, TRUE);

	// Set the scale factors for width and height
	cinfo.scale_num = 2;
	cinfo.scale_denom = 3;

	// Start decompression
	jpeg_start_decompress(&cinfo);

	// Allocate memory for the buffer
	row_stride = cinfo.output_width * cinfo.output_components;
	buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	// Read scanlines and render to the pix array
	int i = 0;
	while (cinfo.output_scanline < cinfo.output_height) {
		jpeg_read_scanlines(&cinfo, buffer, 1);

		// Scale down and assign pixel values
		for (int j = 0; j < width; j++) {
			if (i > 720 * 480) continue;
			int src_index = j * cinfo.output_width / width;
			pix[i].r = buffer[0][src_index * cinfo.output_components];
			pix[i].g = buffer[0][src_index * cinfo.output_components + 1];
			pix[i].b = buffer[0][src_index * cinfo.output_components + 2];
			pix[i].a = 0;  // Assuming no alpha channel in JPEG

			i++;
		}
	}

	// Finish decompression
	jpeg_finish_decompress(&cinfo);

	// Release the JPEG decompression object
	jpeg_destroy_decompress(&cinfo);

	return 1;
}


int main() {
	Tigr* screen = tigrWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello", 0);
	struct PtpRuntime r;
	ptp_generic_init(&r);

	struct PtpDeviceInfo di;

	if (ptp_device_init(&r)) {
		puts("Device connection error");
		return 0;
	}

	int rc = ptp_open_session(&r);

	rc = ptp_get_device_info(&r, &di);

	ptp_eos_set_remote_mode(&r, 1);
	ptp_eos_set_event_mode(&r, 1);

	rc = ptp_liveview_init(&r);
	if (rc) {
		return 0;
	}

	void *lv = malloc(ptp_liveview_size(&r));
	int lv_is_inactive = 0;

	int counter = 0;
	uint32_t *buffer = NULL;
	while (!tigrClosed(screen) && !tigrKeyDown(screen, TK_ESCAPE)) {
		if (counter == 0) {
			rc = ptp_ml_init_bmp_lv(&r);
			if (rc < 0) {
				printf("Error loading ML BMP spec: %d\n", rc);
				break;
			}
		}

		if (lv_is_inactive && (counter & 1)) {
			goto skip_lv;
		}

		rc = ptp_liveview_frame(&r, lv);
		if (rc < 0) {
			printf("Error getting EOS frame: %d\n", rc);
			break;
		} else if (ptp_get_return_code(&r) == PTP_RC_OK) {
			render_jpeg(SCREEN_WIDTH, SCREEN_HEIGHT, screen->pix, lv, rc);
			lv_is_inactive = 0;
		} else if (ptp_get_return_code(&r) == PTP_RC_CANON_NotReady) {
			// Must have just been deactivated, reinit palette
			if (!lv_is_inactive) {
				rc = ptp_ml_init_bmp_lv(&r);
			}

			lv_is_inactive++;
		}

		skip_lv:;

		if (counter >= 15) {
			counter = 0;
		}

		if (lv_is_inactive > 10) {
			for (int i = 0; i < screen->w * screen->h; i++) {
				screen->pix[i].r = 0x0;
				screen->pix[i].g = 0;
				screen->pix[i].b = 0;
				screen->pix[i].a = 0;
			}
		}

		if (!(counter % 4) || lv_is_inactive) {
			if (buffer != NULL) {
				free(buffer);
			}

			rc = ptp_ml_get_bmp_lv(&r, &buffer);
			if (rc) {
				printf("Error getting ML frame: %d\n", rc);
				break;
			}
			if (buffer == NULL) {
				printf("NULL buffer error\n");
				break;
			}
		}

		if (buffer != NULL) {
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
		}

		tigrUpdate(screen);

		counter++;
	}
	
	tigrFree(screen);
	free(lv);
}
