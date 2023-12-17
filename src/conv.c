// Proprietary vendor property data conversion to standard simple formats
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)

#include <stdio.h>
#include <string.h>

#include <camlib.h>

// TODO: Standard MTP scales by 10,000
struct CanonShutterSpeed {
	int value; // value times 100000
	int data; // data from camera
}canon_shutter[] = {
	{0, 0xc}, // BULB 1300D
	{0, 0x4}, // BULB 5dmk3
	{3000000,0x10},
	{2500000,0x13},
	{2000000,0x15},
	{1500000,0x18},
	{1300000,0x1b},
	{1000000,0x1d},
	{800000,0x20},
	{600000,0x23},
	{500000,0x25},
	{400000,0x28},
	{320000,0x2b},
	{250000,0x2d},
	{200000,0x30},
	{160000,0x33},
	{130000,0x35},
	{100000,0x38},
	{80000,0x3b},
	{60000,0x3d},
	{50000,0x40},
	{40000,0x43},
	{30000,0x45},
	{25000,0x48},
	{20000,0x4b},
	{16666,0x4d},
	{12500,0x50},
	{100000 / 10,0x53},
	{100000 / 13,0x55},
	{100000 / 15,0x58},
	{100000 / 20,0x5b},
	{100000 / 25,0x5d},
	{100000 / 30,0x60},
	{100000 / 40,0x63},
	{100000 / 50,0x65},
	{100000 / 60,0x68},
	{100000 / 80,0x6b},
	{100000 / 100,0x6d},
	{100000 / 125,0x70},
	{100000 / 160,0x73},
	{100000 / 200,0x75},
	{100000 / 250,0x78},
	{100000 / 320,0x7b},
	{100000 / 400,0x7d},
	{100000 / 500,0x80},
	{100000 / 640,0x83},
	{100000 / 800,0x85},
	{100000 / 1000,0x88},
	{100000 / 1250,0x8b},
	{100000 / 1600,0x8d},
	{100000 / 2000,0x90},
	{100000 / 2500,0x93},
	{100000 / 3200,0x95},
	{100000 / 4000,0x98},
	{100000 / 5000,0x9a},
	{100000 / 6400,0x9d},
	{100000 / 8000,0xa0},
};

int ptp_eos_get_shutter(int data, int dir) {
	for (int i = 0; i < (int)(sizeof(canon_shutter) / sizeof(struct CanonShutterSpeed)); i++) {
		if (dir) {
			if (canon_shutter[i].value == data) {
				return canon_shutter[i].data;
			}
		} else {
			if (canon_shutter[i].data == data) {
				return canon_shutter[i].value;
			}
		}
	}

	return data;
}

struct CanonISO {
	int value;
	int data;
}canon_iso[] = {
	{0, 0}, // AUTO
	{50, 0x40},
	{100, 0x48},
	{125, 0x4b},
	{160, 0x4d},
	{200, 0x50},
	{250, 0x53},
	{320, 0x55},
	{400, 0x58},
	{500, 0x5b},
	{640, 0x5b},
	{800, 0x60},
	{1000, 0x63},
	{1250, 0x65},
	{1600, 0x68},
	{3200, 0x70},
	{6400, 0x78},
	{12800, 0x78+8},
};

int ptp_eos_get_iso(int data, int dir) {
	for (int i = 0; i < (int)(sizeof(canon_iso) / sizeof(struct CanonISO)); i++) {
		if (dir) {
			if (canon_iso[i].value == data) {
				return canon_iso[i].data;
			}
		} else {
			if (canon_iso[i].data == data) {
				return canon_iso[i].value;
			}
		}
	}

	return data;
}

#if 0
{0, 0} // Undefined
{1, 0} // Manual (RGB gain)
{2, 0} // Automatic
{3, 0} // One push automatic
{4, 1} // Daylight
{5, 4} // Florescent
{6, 3} // Tungsten
{7, 8} // Flash
#endif

struct CanonWhiteBalance {
	int value;
	int data;
}canon_white_balance[] = {
	{0, 0}, // AUTO
	{1, 1}, // Daylight
	{2, 8}, // Shade
	{3, 3}, // Tungsten/Incandescent
	{4, 4}, // White florescent
};

int ptp_eos_get_white_balance(int data, int dir) {
	for (int i = 0; i < (int)(sizeof(struct CanonWhiteBalance) / sizeof(struct CanonWhiteBalance)); i++) {
		if (dir) {
			if (canon_white_balance[i].value == data) {
				return canon_white_balance[i].data;
			}
		} else {
			if (canon_white_balance[i].data == data) {
				return canon_white_balance[i].value;
			}
		}
	}

	return data;
}

struct CanonAperture {
	int value;
	int data;
}canon_aperture[] = {
	{12, 0xd}, // TODO: standard MTP scales this by 100, currently scaled by 10
	{14, 0x10},
	{16, 0x13},
	{18, 0x15},
	{20, 0x18},
	{22, 0x1b},
	{25, 0x1d},
	{28, 0x20},
	{32, 0x23},
	{35, 0x25},
	{40, 0x28},
	{45, 0x2b},
	{50, 0x2d},
	{56, 0x30},
	{63, 0x33},
	{71, 0x35},
	{80, 0x38},
	{90, 0x3b},
	{100, 0x3d},
	{110, 0x40},
	{130, 0x43},
	{140, 0x45},
	{160, 0x48},
	{180, 0x4b},
	{200, 0x4d},
	{220, 0x50},
	{250, 0x53},
	{290, 0x55},
	{320, 0x58},
};

int ptp_eos_get_aperture(int data, int dir) {
	for (int i = 0; i < (int)(sizeof(canon_aperture) / sizeof(struct CanonAperture)); i++) {
		if (dir) {
			if (canon_aperture[i].value == data) {
				return canon_aperture[i].data;
			}
		} else {
			if (canon_aperture[i].data == data) {
				return canon_aperture[i].value;
			}
		}
	}

	return data;
}

// Lots of confusing types (resolutions, raw+jpeg, superfine, etc)
// Converts to camlib wrapper types (enum ImageFormats)
struct CanonImageFormats {
	int value;
	int data[9];
}canon_imgformats[] = {
	{IMG_FORMAT_RAW, {1, 16, 6, 0, 4}}, // RAW
	{IMG_FORMAT_STD, {1, 16, 1, 0, 2}}, // STD
	{IMG_FORMAT_HIGH, {1, 16, 1, 0, 3}}, // HIGH
	{IMG_FORMAT_RAW_JPEG, {2, 16, 6, 0, 4, 16, 1, 0, 3}}, // RAW + HIGH JPG
};

int ptp_eos_get_imgformat_value(int data[5]) {
	for (int i = 0; i < (int)(sizeof(canon_imgformats) / sizeof(struct CanonImageFormats)); i++) {
		if (!memcmp(canon_imgformats[i].data, data, sizeof(int) * 5)) {
			return canon_imgformats[i].value;
		}
	}

	return 0;
}
