#ifndef OPERATIONS_H
#define OPERATIONS_H

int ptp_open_session(struct PtpRuntime *r);
int ptp_close_session(struct PtpRuntime *r);
int ptp_get_device_info(struct PtpRuntime *r, struct PtpDeviceInfo *di);
int ptp_get_storage_ids(struct PtpRuntime *r, struct PtpStorageIds *si);
int ptp_custom_recieve(struct PtpRuntime *r, int code);
int ptp_init_capture(struct PtpRuntime *r, int storage_id, int object_format);
int ptp_get_storage_info(struct PtpRuntime *r, struct PtpStorageInfo *si, int id);
int ptp_get_prop_value(struct PtpRuntime *r, int code);

int ptp_eos_get_viewfinder_data(struct PtpRuntime *r);
int ptp_eos_set_remote_mode(struct PtpRuntime *r, int mode);
int ptp_eos_set_prop_value(struct PtpRuntime *r, int code, int value);
int ptp_eos_set_event_mode(struct PtpRuntime *r, int mode);
int ptp_eos_remote_release_off(struct PtpRuntime *r, int mode);
int ptp_eos_remote_release_on(struct PtpRuntime *r, int mode);
int ptp_eos_get_event(struct PtpRuntime *r);
int ptp_eos_hdd_capacity(struct PtpRuntime *r);
int ptp_eos_get_prop_value(struct PtpRuntime *r, int code);

// steps can be between -3 and 3
int ptp_eos_drive_lens(struct PtpRuntime *r, int steps);
int ptp_eos_ping(struct PtpRuntime *r);

// Generic operation wrappers
int ptp_generic_drive_lens(struct PtpRuntime *r, int x);
int ptp_generic_take_picture(struct PtpRuntime *r);

// Get approx size of liveview, for allocations only
int ptp_liveview_size(struct PtpRuntime *r);

// Initialize the liveview if needed
int ptp_liveview_init(struct PtpRuntime *r);

// Get a frame directly into buffer, can be JPEG or raw data
int ptp_liveview_frame(struct PtpRuntime *r, uint8_t *buffer);

int ptp_liveview_type(struct PtpRuntime *r);

#endif
