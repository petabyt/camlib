// Easy to use operation (OC) functions. Requires bindings.
#include <stddef.h>
#include <string.h>

#include "ptp.h"
#include "canon.h"
#include "backend.h"
#include "camlib.h"

int ptp_open_session(struct PtpRuntime *r) {
	r->session++;

	struct PtpCommand cmd;
	cmd.code = PTP_OC_OpenSession;
	cmd.params[0] = r->session;
	cmd.param_length = 1;
	cmd.data_length = 0;

	int length = ptp_new_cmd_packet(r, &cmd);

	// PTP open session transaction ID is always 0
	r->transaction = 0;

	if (ptp_send_bulk_packets(r, length) != length) return -1;

	// Set transaction ID back to start
	r->transaction = 1;

	ptp_recieve_bulk_packets(r);
	return 0;
}

int ptp_close_session(struct PtpRuntime *r) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_CloseSession;
	cmd.param_length = 0;

	int length = ptp_new_cmd_packet(r, &cmd);
	ptp_send_bulk_packets(r, length);
	ptp_recieve_bulk_packets(r);
	return 0;
}

int ptp_get_device_info(struct PtpRuntime *r, struct PtpDeviceInfo *di) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_GetDeviceInfo;
	cmd.param_length = 0;

	int length = ptp_new_data_packet(r, &cmd);
	if (ptp_send_bulk_packets(r, length) != length) return -1;
	int x = ptp_recieve_bulk_packets(r);
    if (x <= 0) return -2;

	return ptp_parse_device_info(r, di);
}

int ptp_get_storage_ids(struct PtpRuntime *r, struct PtpStorageIds *si) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_GetStorageIDs;
	cmd.param_length = 0;

	int length = ptp_new_data_packet(r, &cmd);
	ptp_send_bulk_packets(r, length);
	ptp_recieve_bulk_packets(r);

	memcpy(si, r->data, sizeof(struct PtpStorageIds));

	return 0;
}

int ptp_get_storage_info(struct PtpRuntime *r, struct PtpStorageInfo *si) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_GetStorageInfo;
	cmd.param_length = 0;

	int length = ptp_new_data_packet(r, &cmd);
	ptp_send_bulk_packets(r, length);
	ptp_recieve_bulk_packets(r);

	memcpy(si, r->data, sizeof(struct PtpStorageInfo));

	return 0;
}

int ptp_get_num_objects(struct PtpRuntime *r) {
	
}
