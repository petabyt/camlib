// Liveview wrappers - headers defined in operations.h
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)

// TODO: ptp_liveview_jpeg will return pointer to jpeg and length

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <camlib.h>
#include <ptp.h>

#define MAX_EOS_JPEG_SIZE 500000

#define PTP_OC_ML_Live360x240 0x9997
#define PTP_ML_LvWidth 360
#define PTP_ML_LvHeight 240

#ifndef ML_TRANSPARENCY_PIXEL
#define ML_TRANSPARENCY_PIXEL 0x0
#endif

// For debugging
//#define NO_ML_LV

int ptp_liveview_type(struct PtpRuntime *r) {
	int type = ptp_device_type(r);
	if (type == PTP_DEV_CANON || type == PTP_DEV_EOS) {
		#ifndef NO_ML_LV
		if (ptp_check_opcode(r, PTP_OC_ML_Live360x240)) {
			return PTP_LV_ML;
		}
		#endif

		if (ptp_check_opcode(r, PTP_OC_EOS_GetViewFinderData)) {
			return PTP_LV_EOS;
		}

		if (ptp_check_opcode(r, PTP_OC_CANON_GetViewFinderImage)) {
			return PTP_LV_CANON;
		}
	}

	return PTP_LV_NONE;
}

int ptp_liveview_size(struct PtpRuntime *r) {
	switch (ptp_liveview_type(r)) {
	case PTP_LV_ML:
		return PTP_ML_LvWidth * PTP_ML_LvHeight * 4;
	case PTP_LV_EOS:
		return MAX_EOS_JPEG_SIZE;
	}

	return 0;
}

int ptp_liveview_ml(struct PtpRuntime *r, uint8_t *buffer) {
    int a = ptp_custom_recieve(r, PTP_OC_ML_Live360x240);
    if (a < 0) {
        return PTP_IO_ERR;
    } else if (ptp_get_return_code(r) != PTP_RC_OK) {
        return PTP_CHECK_CODE;
    }

    uint8_t *data = ptp_get_payload(r);
    int length = (PTP_ML_LvWidth * PTP_ML_LvHeight);

    for (int i = 0; i < length; i++) {
        buffer[0] = data[0];
        buffer[1] = data[1];
        buffer[2] = data[2];
        buffer[3] = ML_TRANSPARENCY_PIXEL;
        buffer += 4;
        data += 3;
    }

    return length * 4;
}

// TODO: Random faults
int ptp_liveview_eos(struct PtpRuntime *r, uint8_t *buffer) {
	int x = ptp_eos_get_viewfinder_data(r);
	if (x < 0) return x;

	if (ptp_get_return_code(r) == PTP_RC_CANON_NotReady) {
		return 0;
	}

	struct PtpEOSViewFinderData *vfd = (struct PtpEOSViewFinderData *)(ptp_get_payload(r));
	if (MAX_EOS_JPEG_SIZE < vfd->length) {
		return 0;
	}

	memcpy(buffer, ptp_get_payload(r) + 8, vfd->length);
	return vfd->length;
}

int ptp_liveview_init(struct PtpRuntime *r) {
	int x;
	switch (ptp_liveview_type(r)) {
	case PTP_LV_ML:
		return 0;
	case PTP_LV_EOS:
		x = ptp_eos_set_prop_value(r, PTP_PC_EOS_VF_Output, 3);
		if (x) return x;
		x = ptp_eos_set_prop_value(r, PTP_PC_EOS_CaptureDestination, 4);
		if (x) return x;
		//if (ptp_eos_set_prop_value(r, PTP_PC_EOS_EVFMode, 1)) return PTP_CAM_ERR;
		return 0;
	}

	return 1;
}

int ptp_liveview_deinit(struct PtpRuntime *r) {
	int x;
	switch (ptp_liveview_type(r)) {
	case PTP_LV_ML:
		return 0;
	case PTP_LV_EOS:
		x = ptp_eos_set_prop_value(r, PTP_PC_EOS_VF_Output, 0);
		if (x) return x;
		x = ptp_eos_set_prop_value(r, PTP_PC_EOS_CaptureDestination, 2);
		if (x) return x;
		return 0;
	}

	return 1;
}

int ptp_liveview_frame(struct PtpRuntime *r, void *buffer) {
	switch (ptp_liveview_type(r)) {
	case PTP_LV_ML:
		return ptp_liveview_ml(r, (uint8_t *)buffer);
	case PTP_LV_EOS:
		return ptp_liveview_eos(r, (uint8_t *)buffer);
	}

	return PTP_UNSUPPORTED;
}
