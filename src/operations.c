// Easy to use operation (OC) functions. Requires bindings.
#include <stddef.h>
#include <string.h>

#include "ptp.h"
#include "canon.h"
#include "backend.h"
#include "camlib.h"

int ptp_custom_recieve(struct PtpRuntime *r, int code) {
	struct PtpCommand cmd;
	cmd.code = code;
	cmd.param_length = 0;

	int length = ptp_new_cmd_packet(r, &cmd);
	if (ptp_send_bulk_packets(r, length) != length) return -1;
	int x = ptp_recieve_bulk_packets(r);
	if (x <= 0) return -2;
	return x;
}

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
	r->di = di;

	struct PtpCommand cmd;
	cmd.code = PTP_OC_GetDeviceInfo;
	cmd.param_length = 0;

	int length = ptp_new_cmd_packet(r, &cmd);
	if (ptp_send_bulk_packets(r, length) != length) return PTP_IO_ERR;
	if (ptp_recieve_bulk_packets(r) < 0) return PTP_IO_ERR;

	return ptp_parse_device_info(r, di);
}

int ptp_init_capture(struct PtpRuntime *r, int storage_id, int object_format) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_InitiateCapture;
	cmd.param_length = 2;
	cmd.params[0] = storage_id;
	cmd.params[0] = object_format;

	int length = ptp_new_cmd_packet(r, &cmd);
	if (ptp_send_bulk_packets(r, length) != length) return PTP_IO_ERR;
	if (ptp_recieve_bulk_packets(r) < 0) return PTP_IO_ERR;	
	return 0;
}

int ptp_get_storage_ids(struct PtpRuntime *r, struct PtpStorageIds *si) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_GetStorageIDs;
	cmd.param_length = 0;

	int length = ptp_new_cmd_packet(r, &cmd);
	if (ptp_send_bulk_packets(r, length) != length) return PTP_IO_ERR;
	if (ptp_recieve_bulk_packets(r) < 0) return PTP_IO_ERR;

	memcpy(si, ptp_get_payload(r), sizeof(struct PtpStorageIds));

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

int ptp_get_object_info(struct PtpRuntime *r, uint32_t handle) {
	struct PtpObjectInfo oi;

	struct PtpCommand cmd;
	cmd.code = PTP_OC_GetObjectInfo;
	cmd.param_length = 1;

	int length = ptp_new_cmd_packet(r, &cmd);
	if (ptp_send_bulk_packets(r, length) != length) return PTP_IO_ERR;
	if (ptp_recieve_bulk_packets(r) < 0) return PTP_IO_ERR;

	memcpy(&oi, ptp_get_payload(r), sizeof(struct PtpStorageIds));

	return 0;
}

int ptp_get_object_handles(struct PtpRuntime *r, int id, int format_code, int handle) {
	
}

int ptp_get_num_objects(struct PtpRuntime *r) {
	
}
