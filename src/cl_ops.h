/** \file */
#ifndef OPERATIONS_H
#define OPERATIONS_H

/// @brief Set a generic property - abstraction over SetDeviceProp
/// @note May reject writes if an invalid property is found (see event code)
/// @memberof PtpRuntime
int ptp_set_generic_property(struct PtpRuntime *r, char *name, int value);

/// @brief Call before taking a picture - this is generally for 'focusing'
/// On some cameras this does nothing.
/// @note This is meant for a onMouseDown-like event. ptp_take_picture should be called on onMouseUp
/// @memberof PtpRuntime
int ptp_pre_take_picture(struct PtpRuntime *r);

/// @brief Call after calling ptp_pre_take_picture - this time a picture will be taken.
/// @memberof PtpRuntime
int ptp_take_picture(struct PtpRuntime *r);

/// @brief Open a new session - required for most commands
/// @memberof PtpRuntime
int ptp_open_session(struct PtpRuntime *r);

/// @memberof PtpRuntime
int ptp_close_session(struct PtpRuntime *r);

/// @memberof PtpRuntime
int ptp_get_device_info(struct PtpRuntime *r, struct PtpDeviceInfo *di);

/// @brief Returns allocated array of storage IDs
/// call free() afterwards
int ptp_get_storage_ids(struct PtpRuntime *r, struct PtpArray **a);

/// @memberof PtpRuntime
int ptp_init_capture(struct PtpRuntime *r, int storage_id, int object_format);

/// @memberof PtpRuntime
int ptp_init_open_capture(struct PtpRuntime *r, int storage_id, int object_format);

/// @memberof PtpRuntime
int ptp_terminate_open_capture(struct PtpRuntime *r, int trans);

/// @memberof PtpRuntime
int ptp_get_storage_info(struct PtpRuntime *r, int id, struct PtpStorageInfo *si);

/// @memberof PtpRuntime
int ptp_send_object_info(struct PtpRuntime *r, int storage_id, int handle, struct PtpObjectInfo *oi);

/// @memberof PtpRuntime
int ptp_get_prop_value(struct PtpRuntime *r, int code);

/// @memberof PtpRuntime
int ptp_set_prop_value(struct PtpRuntime *r, int code, int value);

/// @memberof PtpRuntime
int ptp_set_prop_value_data(struct PtpRuntime *r, int code, void *data, int length);

/// @memberof PtpRuntime
int ptp_get_prop_desc(struct PtpRuntime *r, int code, struct PtpPropDesc *pd);

/// @brief Gets a list of object handles in a storage device or folder.
// @param id storage ID
// @param format Can specify file format ID, or zero for all IDs
// @param in Can be folder object ID, or 0 for recursive (entire filesystem)
// @param[out] a Output array is a pointer to data packet, and will be overwritten by new operations
/// @memberof PtpRuntime
int ptp_get_object_handles(struct PtpRuntime *r, int id, int format, int in, struct PtpArray **a);

/// @memberof PtpRuntime
int ptp_get_object_info(struct PtpRuntime *r, uint32_t handle, struct PtpObjectInfo *oi);

/// @memberof PtpRuntime
int ptp_move_object(struct PtpRuntime *r, int storage_id, int handle, int folder);

/// @memberof PtpRuntime
int ptp_delete_object(struct PtpRuntime *r, int handle, int format_code);

/// @brief Raw JPEG data is accessible from ptp_get_payload()
/// @note Not thread safe.
/// @memberof PtpRuntime
int ptp_get_thumbnail(struct PtpRuntime *r, int handle);

/// @note Not thread safe.
/// @memberof PtpRuntime
int ptp_get_partial_object(struct PtpRuntime *r, uint32_t handle, int offset, int max);

/// @brief Download an object
/// @memberof PtpRuntime
int ptp_get_object(struct PtpRuntime *r, int handle);

/// @brief Download an object from handle, to a local file (uses GetPartialObject)
/// @memberof PtpRuntime
int ptp_download_object(struct PtpRuntime *r, int handle, FILE *stream, size_t max);

/// @brief Recieve a generic list of all properties received in DeviceInfo
/// This is similar to getting all events, but for first startup when you know nothing.
/// Some vendors do this, but this gets all the properties manually.
/// @param[out] s Output structure, caller must free
/// @memberof PtpRuntime
int ptp_get_all_known(struct PtpRuntime *r, struct PtpGenericEvent **s, int *length);

/// @note PTP/IP only
/// @memberof PtpRuntime
int ptpip_init_events(struct PtpRuntime *r);

/// @note PTP/IP only
/// @memberof PtpRuntime
int ptpip_init_command_request(struct PtpRuntime *r, char *device_name);

// EOS Only functions - not for Canon point and shoot
int ptp_eos_get_viewfinder_data(struct PtpRuntime *r);
int ptp_eos_set_remote_mode(struct PtpRuntime *r, int mode);
int ptp_eos_set_prop_value(struct PtpRuntime *r, int code, int value);
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

int ptp_chdk_get_version(struct PtpRuntime *r);
int ptp_chdk_upload_file(struct PtpRuntime *r, char *input, char *dest);

// Canon advanced extensions - available in canon-adv.c
int ptp_eos_activate_command(struct PtpRuntime *r);
int ptp_eos_exec_evproc(struct PtpRuntime *r, void *data, int length, int expect_return);
int ptp_eos_evproc_run(struct PtpRuntime *r, char *fmt, ...);
int ptp_eos_evproc_return_data(struct PtpRuntime *r);
int ptp_eos_fa_get_build_version(struct PtpRuntime *r, char *buffer, int max);

#endif
