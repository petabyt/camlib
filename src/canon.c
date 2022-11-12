#include <stddef.h>
#include <string.h>

#include "ptp.h"
#include "canon.h"
#include "backend.h"
#include "camlib.h"

int ptp_eos_remote_release_on(struct PtpRuntime *r, int mode) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_EOS_RemoteReleaseOn;
	cmd.param_length = 2;
	cmd.params[0] = mode;
	cmd.params[1] = 0;

	return ptp_generic_send(r, &cmd);
}

// BROKEN
int ptp_canon_get_viewfinder_data(struct PtpRuntime *r) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_EOS_GetViewFinderData;
	cmd.param_length = 3;
	
	cmd.params[0] = 0;
	cmd.params[1] = 0x3e3f0000;
	cmd.params[2] = 0xd96636c;

	return ptp_generic_send(r, &cmd);
}
