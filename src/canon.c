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

	int length = ptp_new_cmd_packet(r, &cmd);
	if (ptp_send_bulk_packets(r, length) != length) return PTP_IO_ERR;
	if (ptp_recieve_bulk_packets(r) < 0) return PTP_IO_ERR;
	return 0;
}

// BROKEN
int ptp_canon_get_viewfinder_data(struct PtpRuntime *r) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_EOS_GetViewFinderData;
	cmd.param_length = 3;
	
	cmd.params[0] = 0;
	cmd.params[1] = 0x3e3f0000;
	cmd.params[2] = 0xd96636c;

	int length = ptp_new_cmd_packet(r, &cmd);
	if (ptp_send_bulk_packets(r, length) != length) return -1;
	int x = ptp_recieve_bulk_packets(r);
	if (x <= 0) return -2;
	return x;
}
