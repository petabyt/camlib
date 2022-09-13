// Easy to use operation (OC) functions. Requires bindings.
#include <stddef.h>
#include <string.h>
#include "ptp.h"
#include "bindings.h"
#include "piclib.h"
#include "deviceinfo.h"

int ptp_recv_bulk(struct PtpRuntime *r, struct PtpCommand *cmd) {
	cmd->data_length = 0;
	int length = ptp_bulk_packet_cmd(r, cmd);
	ptp_send_bulk_packet(r, length);
	length = ptp_bulk_packet_data(r, cmd);
	ptp_send_bulk_packet(r, length);
	return length;
}

int ptp_send_bulk(struct PtpRuntime *r, struct PtpCommand *cmd) {
	int length = ptp_bulk_packet_cmd(r, cmd);
	ptp_send_bulk_packet(r, length);
	length = ptp_bulk_packet_data(r, cmd);
	return length;
}

void ptp_update_data_length(struct PtpRuntime *r, int length) {
	struct PtpBulkContainer *bulk = (struct PtpBulkContainer*)(r->data);
	bulk->length = length;
}

int ptp_get_device_info(struct PtpRuntime *r, struct PtpDeviceInfo *di) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_GetDeviceInfo;
	cmd.param_length = 0;

	ptp_recv_bulk(r, &cmd);
	ptp_recieve_bulk_packets(r);

	return ptp_parse_device_info(r, di);
}

int ptp_canon_evproc(struct PtpRuntime *r, char *string) {
	struct PtpCommand cmd;
	cmd.param_length = 0;
	cmd.code = PTP_OC_Canon_ExecEventProc;

	int length = ptp_bulk_packet_data(r, &cmd);
	length += ptp_wide_string((char*)(r->data + length), 100, string);
	memset(r->data + length, 0, 50);

	ptp_update_data_length(r, length);

	ptp_send_bulk(r, &cmd);
	ptp_recieve_bulk_packets(r);
}
