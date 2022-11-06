#include <stddef.h>
#include <string.h>

#include "ptp.h"
#include "canon.h"
#include "backend.h"
#include "camlib.h"

int ptp_canon_evproc(struct PtpRuntime *r, char *string) {
	struct PtpCommand cmd;
	cmd.param_length = 0;
	cmd.code = PTP_OC_CANON_ExecuteEventProc;

	int length = ptp_new_data_packet(r, &cmd);
	length += ptp_wide_string((char*)(r->data + length), 100, string);

	// Append zeros to bypass EvProc parameters
	memset(r->data + length, 0, 30);

	ptp_update_data_length(r, length + 30);
	return ptp_send_bulk_packets(r, length);
}
