#include <stdlib.h>
#include <stdio.h>
#include <jni.h>
#include <string.h>

#include <ptp.h>
#include <camlib.h>
#include <operations.h>

#define PTP_OC_ML_Live360x240 0x9997

int ptp_liveview_type(struct PtpRuntime *r) {
	if (ptp_detect_device(r) == PTP_DEV_CANON) {
		if (ptp_check_opcode(r, PTP_OC_ML_Live360x240)) {
			return PTP_LV_ML;
		}

		if (ptp_check_opcode(r, PTP_OC_EOS_GetViewFinderData)) {
			return PTP_LV_EOS;
		}

		if (ptp_check_opcode(r, PTP_OC_CANON_GetViewFinderImage)) {
			return PTP_LV_CANON;
		}
	}

	return PTP_LV_NONE;
}

int ptp_liveview_ml(struct PtpRuntime *r, uint8_t *buffer) {
	int a = ptp_custom_recieve(r, PTP_OC_ML_Live360x240);
	if (a < 0) {
		return PTP_IO_ERR;
	} else if (ptp_get_return_code(r) != PTP_RC_OK) {
		return PTP_CAM_ERR;
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

int ptp_liveview_eos(struct PtpRuntime *r, uint8_t *buffer) {
	return ptp_eos_get_viewfinder_data(&r);
}

int ptp_liveview_eos_init(struct PtpRuntime *r) {
	if (ptp_eos_set_event_mode(&r, 1)) return PTP_CAM_ERR;
	if (ptp_eos_set_remote_mode(&r, 1)) return PTP_CAM_ERR;
	if (ptp_eos_set_prop_value(&r, PTP_PC_CANON_EOS_VF_Output, 3)) return PTP_CAM_ERR;
	if (ptp_eos_set_prop_value(&r, PTP_PC_CANON_EOS_EVFMode, 1)) return PTP_CAM_ERR;
	if (ptp_eos_set_prop_value(&r, PTP_PC_EOS_CaptureDest, 4)) return PTP_CAM_ERR;

	return 0;
}

int ptp_liveview_init(struct PtpRuntime *r, uint8_t *buffer) {
	switch (ptp_liveview_type(r)) {
	case PTP_LV_ML:
		return 0;
	case PTP_LV_EOS:
		ptp_liveview_eos_init();
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

	return 1;
}
