// Easy to use operation (OC) functions. Requires bindings.
#include <stddef.h>
#include <string.h>

#include "ptp.h"
#include "backend.h"
#include "camlib.h"

// Decide cmd then data or mixed (??)

// Send the initial "cmd" packet (1), build the data packet (2), then leave
// caller the option to modify data packet before sending it off
int ptp_prep_recv_bulk(struct PtpRuntime *r, struct PtpCommand *cmd) {
	cmd->data_length = 0;
	int length = ptp_bulk_packet_cmd(r, cmd);
	ptp_send_bulk_packets(r, length);
	length = ptp_bulk_packet_data(r, cmd);
	ptp_send_bulk_packets(r, length);
	return length;
}

int ptp_prep_send_bulk(struct PtpRuntime *r, struct PtpCommand *cmd) {
	int length = ptp_bulk_packet_cmd(r, cmd);
	ptp_send_bulk_packets(r, length);

	length = ptp_bulk_packet_data(r, cmd);
	return length;
}

void ptp_update_data_length(struct PtpRuntime *r, int length) {
	struct PtpBulkContainer *bulk = (struct PtpBulkContainer*)(r->data);
	bulk->length = length;
}

void ptp_update_transaction(struct PtpRuntime *r, int t) {
	struct PtpBulkContainer *bulk = (struct PtpBulkContainer*)(r->data);
	bulk->transaction = t;
}

int ptp_open_session(struct PtpRuntime *r) {
	r->session++;

	struct PtpCommand cmd;
	cmd.code = PTP_OC_OpenSession;
	cmd.params[0] = r->session;
	cmd.param_length = 1;
	cmd.data_length = 0;

	int length = ptp_bulk_packet_cmd(r, &cmd);

	// PTP open session transaction ID is always 0
	r->transaction = 0;

	ptp_send_bulk_packets(r, length);

	// Set transaction ID back to start
	r->transaction = 1;

	ptp_recieve_bulk_packets(r);
	return 0;
}

int ptp_close_session(struct PtpRuntime *r) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_CloseSession;
	cmd.param_length = 0;

	int length = ptp_bulk_packet_cmd(r, &cmd);
	ptp_send_bulk_packets(r, length);
	ptp_recieve_bulk_packets(r);
	return 0;
}

int ptp_get_device_info(struct PtpRuntime *r, struct PtpDeviceInfo *di) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_GetDeviceInfo;
	cmd.param_length = 0;

	int length = ptp_bulk_packet_cmd(r, &cmd);
	if (ptp_send_bulk_packets(r, length) != length) return 0;
	ptp_recieve_bulk_packets(r);
	return ptp_parse_device_info(r, di);
}

int ptp_get_storage_ids(struct PtpRuntime *r, struct PtpStorageIds *si) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_GetStorageIDs;
	cmd.param_length = 0;

	int length = ptp_bulk_packet_cmd(r, &cmd);
	ptp_send_bulk_packets(r, length);
	ptp_recieve_bulk_packets(r);

	memcpy(si, r->data, sizeof(struct PtpStorageIds));

	return 0;
}

int ptp_get_storage_info(struct PtpRuntime *r, struct PtpStorageInfo *si) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_GetStorageInfo;
	cmd.param_length = 0;

	int length = ptp_bulk_packet_cmd(r, &cmd);
	ptp_send_bulk_packets(r, length);
	ptp_recieve_bulk_packets(r);

	memcpy(si, r->data, sizeof(struct PtpStorageInfo));

	return 0;
}

int ptp_get_num_objects(struct PtpRuntime *r) {
	
}

int ptp_canon_evproc(struct PtpRuntime *r, char *string) {
	struct PtpCommand cmd;
	cmd.param_length = 0;
	cmd.code = PTP_OC_CANON_ExecuteEventProc;

	int length = ptp_bulk_packet_data(r, &cmd);
	length += ptp_wide_string((char*)(r->data + length), 100, string);
	memset(r->data + length, 0, 50);

	ptp_update_data_length(r, length);

	ptp_prep_send_bulk(r, &cmd);
	ptp_send_bulk_packets(r, length);
	return 0;
}
