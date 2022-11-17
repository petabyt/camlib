#include <stddef.h>
#include <string.h>

#include <ptp.h>
#include <backend.h>
#include <camlib.h>

int ptp_generic_take_picture(struct PtpRuntime *r) {
	if (ptp_check_opcode(r, PTP_OC_InitiateCapture)) {
		return ptp_init_capture(r, 0, 0);
	} else if (ptp_detect_device(r) == PTP_DEV_CANON) {
		int x = ptp_eos_remote_release_on(r, 1)
		x |= ptp_eos_remote_release_on(r, 2);
		x |= ptp_eos_remote_release_off(r, 1);
		x |= ptp_eos_remote_release_off(r, 2);
		if (x) {
			return PTP_CAM_ERR;
		}
	}

	return PTP_CAM_ERR;
}

int ptp_generic_set_property(struct PtpRuntime *r) {
	
}

int ptp_generic_drive_lens(struct PtpRuntime *r) {
	ptp_eos_drive_lens(&r, 0x0002);
}
