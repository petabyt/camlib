#ifndef OPERATIONS_H
#define OPERATIONS_H

int ptp_open_session(struct PtpRuntime *r);
int ptp_close_session(struct PtpRuntime *r);
int ptp_get_device_info(struct PtpRuntime *r, struct PtpDeviceInfo *di);
int ptp_get_storage_ids(struct PtpRuntime *r, struct PtpStorageIds *si);
int ptp_custom_recieve(struct PtpRuntime *r, int code);
int ptp_init_capture(struct PtpRuntime *r, int storage_id, int object_format);
int ptp_get_storage_info(struct PtpRuntime *r, struct PtpStorageInfo *si, int id);

int ptp_canon_get_viewfinder_data(struct PtpRuntime *r);
int ptp_canon_evproc(struct PtpRuntime *r, char *string);

int ptp_liveview_frame(struct PtpRuntime *r, uint8_t *buffer);
#endif
