#ifndef OPERATIONS_H
#define OPERATIONS_H

// Standard PTP operation code functions

// Note that opening a session is required for most vendor commands
int ptp_open_session(struct PtpRuntime *r);
int ptp_close_session(struct PtpRuntime *r);
int ptp_get_device_info(struct PtpRuntime *r, struct PtpDeviceInfo *di);
// Recieves storage IDs into an UintArray. The 'pointer pointer' will be assigned to the r->data
// buffer, so it will eventually be overwritten. You can duplicate the array with ptp_dup_uint_array()
int ptp_get_storage_ids(struct PtpRuntime *r, struct UintArray **a);
int ptp_init_capture(struct PtpRuntime *r, int storage_id, int object_format);
int ptp_init_open_capture(struct PtpRuntime *r, int storage_id, int object_format);
int ptp_terminate_open_capture(struct PtpRuntime *r, int trans);
int ptp_get_storage_info(struct PtpRuntime *r, int id, struct PtpStorageInfo *si);
int ptp_send_object_info(struct PtpRuntime *r, int storage_id, int handle, struct PtpObjectInfo *oi);
int ptp_get_prop_value(struct PtpRuntime *r, int code);
int ptp_set_prop_value(struct PtpRuntime *r, int code, int value);
int ptp_get_prop_desc(struct PtpRuntime *r, int code, struct PtpDevPropDesc *pd);
// Gets a list of object handles in a storage device or folder.
// id: storage ID
// format: Can specify file format ID, or zero for all
// in: Can be folder object ID, or 0 for recursive all
// Output array is a pointer to data packet, and will be overwritten by new operations
int ptp_get_object_handles(struct PtpRuntime *r, int id, int format, int in, struct UintArray **a);
int ptp_get_object_info(struct PtpRuntime *r, uint32_t handle, struct PtpObjectInfo *oi);
int ptp_move_object(struct PtpRuntime *r, int storage_id, int handle, int folder);
int ptp_delete_object(struct PtpRuntime *r, int handle, int format_code);
// Raw JPEG data is accessible from ptp_get_payload()
int ptp_get_thumbnail(struct PtpRuntime *r, int handle);
int ptp_get_partial_object(struct PtpRuntime *r, uint32_t handle, int offset, int max);
// Download an object from handle, to a local file (uses GetPartialObject)
int ptp_download_file(struct PtpRuntime *r, int handle, char *file);
// For debugging only
int ptp_custom_recieve(struct PtpRuntime *r, int code);

// EOS Only functions - not for Canon point and shoot
int ptp_eos_get_viewfinder_data(struct PtpRuntime *r);
int ptp_eos_set_remote_mode(struct PtpRuntime *r, int mode);
int ptp_eos_set_prop_value(struct PtpRuntime *r, int code, int value);
int ptp_eos_set_prop_data(struct PtpRuntime *r, int code, void *data, int dlength);
int ptp_eos_set_event_mode(struct PtpRuntime *r, int mode);
int ptp_eos_remote_release_off(struct PtpRuntime *r, int mode);
int ptp_eos_remote_release_on(struct PtpRuntime *r, int mode);
int ptp_eos_get_event(struct PtpRuntime *r);
int ptp_eos_hdd_capacity_push(struct PtpRuntime *r);
int ptp_eos_hdd_capacity_pop(struct PtpRuntime *r);
int ptp_eos_get_prop_value(struct PtpRuntime *r, int code);
int ptp_eos_bulb_start(struct PtpRuntime *r);
int ptp_eos_bulb_stop(struct PtpRuntime *r);
int ptp_eos_set_ui_lock(struct PtpRuntime *r);
int ptp_eos_reset_ui_lock(struct PtpRuntime *r);
int ptp_eos_cancel_af(struct PtpRuntime *r);
// steps can be between -3 and 3
int ptp_eos_drive_lens(struct PtpRuntime *r, int steps);
int ptp_eos_ping(struct PtpRuntime *r);

// Get max expected size of liveview, for allocations only
int ptp_liveview_size(struct PtpRuntime *r);
// Runs vendor-specific commands for the liveview
int ptp_liveview_init(struct PtpRuntime *r);
int ptp_liveview_deinit(struct PtpRuntime *r);
// Get a frame directly into a buffer. Size is expected to be from ptp_liveview_size()
int ptp_liveview_frame(struct PtpRuntime *r, void *buffer);
int ptp_liveview_type(struct PtpRuntime *r);

// Get Magic Lantern transparent menus buffer - see https://github.com/petabyt/ptpview
int ptp_ml_init_bmp_lv(struct PtpRuntime *r);
int ptp_ml_get_bmp_lv(struct PtpRuntime *r, uint32_t **buffer_ptr);

// Fuji vendor version of SendObjectInfo - same as standard, but no parameters
int ptp_fuji_send_object_info(struct PtpRuntime *r, struct PtpObjectInfo *oi);
int ptp_fuji_send_object(struct PtpRuntime *r, struct PtpObjectInfo *oi, void *data, int length);

// Fujifilm IP-only functions
int ptpip_fuji_init(struct PtpRuntime *r, char *device_name);
int ptp_fuji_ping(struct PtpRuntime *r);
int ptpip_fuji_wait_unlocked(struct PtpRuntime *r);
int ptpip_fuji_get_object_info(struct PtpRuntime *r, uint32_t handle, struct PtpFujiObjectInfo *oi);

#endif
