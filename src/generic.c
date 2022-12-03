// Generic front-end functions

#include <stddef.h>
#include <string.h>

#include <ptp.h>
#include <backend.h>
#include <camlib.h>
#include <operations.h>

#if 0
int ptp_wait_events(struct PtpRuntime *r) {
	int dev = ptp_detect_device(r);
	if (dev == PTP_DEV_CANON) {
		do {
			if (ptp_eos_get_event(r)) return PTP_IO_ERR;
		} while (ptp_get_return_code(r) != PTP_RC_OK);
	}

	int code = -1;
}
#endif

int ptp_generic_take_picture(struct PtpRuntime *r) {
	if (ptp_check_opcode(r, PTP_OC_InitiateCapture)) {
		return ptp_init_capture(r, 0, 0);
	} else if (ptp_detect_device(r) == PTP_DEV_CANON) {
		int x = ptp_eos_remote_release_on(r, 1);
		x |= ptp_eos_remote_release_on(r, 2);
		x |= ptp_eos_remote_release_off(r, 2);
		x |= ptp_eos_remote_release_off(r, 1);
		if (x) {
			return PTP_CAM_ERR;
		}
	} else {
		return PTP_CAM_ERR; // ?
	}

	return 0;
}

int ptp_generic_set_property(struct PtpRuntime *r) {
	return 0;
}

int ptp_generic_drive_lens(struct PtpRuntime *r, int x) {
	ptp_eos_drive_lens(r, x);
}
