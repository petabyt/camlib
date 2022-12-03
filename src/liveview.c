// Liveview wrappers - headers defined in operations.h
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <ptp.h>
#include <camlib.h>
#include <operations.h>

/*
This interface doesn't feel right, maybe
ptp_liveview_request(r);
char *buffer = malloc(ptp_liveview_get_size(r));
ptp_liveview_decode(r, buffer);
*/

// For debugging
#define NO_ML_LV

#define PTP_OC_ML_Live360x240 0x9997
#define PTP_ML_LvWidth 360
#define PTP_ML_LvHeight 240

int ptp_liveview_type(struct PtpRuntime *r) {
	if (ptp_detect_device(r) == PTP_DEV_CANON) {
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
		return 270000; // ??? (compressed JPG)
	}
}

int ptp_liveview_ml(struct PtpRuntime *r, uint8_t *buffer) {
	int a = ptp_custom_recieve(r, PTP_OC_ML_Live360x240);
	if (a < 0) {
		return PTP_IO_ERR;
	} else if (ptp_get_return_code(r) != PTP_RC_OK) {
		return PTP_CAM_ERR;
	}

	uint8_t *data = ptp_get_payload(r);
	int length = (PTP_ML_LvWidth * PTP_ML_LvHeight);

	for (int i = 0; i < length; i++) {
		buffer[0] = data[0];
		buffer[1] = data[1];
		buffer[2] = data[2];
		buffer[3] = 0x0;
		buffer += 4;
		data += 3;
	}

	return length * 4;
}

int ptp_liveview_eos(struct PtpRuntime *r, uint8_t *buffer) {
	int x = ptp_eos_get_viewfinder_data(r);
	if (x < 0) return x;

	struct PtpEOSViewFinderData *vfd = (struct PtpEOSViewFinderData *)(ptp_get_payload(r));
	memcpy(buffer, ptp_get_payload(r), vfd->length);
	return vfd->length;
}

int ptp_liveview_eos_init(struct PtpRuntime *r) {
	if (ptp_eos_set_event_mode(r, 1)) return PTP_CAM_ERR;
	if (ptp_eos_set_remote_mode(r, 1)) return PTP_CAM_ERR;
	if (ptp_eos_set_prop_value(r, PTP_PC_CANON_EOS_VF_Output, 3)) return PTP_CAM_ERR;
	if (ptp_eos_set_prop_value(r, PTP_PC_CANON_EOS_EVFMode, 1)) return PTP_CAM_ERR;
	if (ptp_eos_set_prop_value(r, PTP_PC_EOS_CaptureDestination, 4)) return PTP_CAM_ERR;

	return 0;
}

int ptp_liveview_init(struct PtpRuntime *r) {
	switch (ptp_liveview_type(r)) {
	case PTP_LV_ML:
		return 0;
	case PTP_LV_EOS:
		return ptp_liveview_eos_init(r);
	}

	return 1;
}

int ptp_liveview_frame(struct PtpRuntime *r, uint8_t *buffer) {
	switch (ptp_liveview_type(r)) {
	case PTP_LV_ML:
		return ptp_liveview_ml(r, buffer);
	case PTP_LV_EOS:
		return ptp_liveview_eos(r, buffer);
	}

	return PTP_CAM_ERR;
}
