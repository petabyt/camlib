/** \file */
// Main header file for Camlib
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)
#ifndef PTP_LIB_H
#define PTP_LIB_H

#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

#include "ptp.h"

// Max timeout for command read/writes
#define PTP_TIMEOUT 1000

// How much ms to wait for r->wait_for_response
#define CAMLIB_WAIT_MS 1000

// Conforms to POSIX 2001, some compilers may not have it
#ifndef CAMLIB_SLEEP
	#include <unistd.h>
	//int usleep(unsigned int usec);
	#define CAMLIB_SLEEP(ms) usleep(ms * 1000)
#endif

#ifdef BUILD_DLL
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_EXPORT
#endif

// Logging+panic mechanism, define it yourself or link in log.c
void ptp_verbose_log(char *fmt, ...);
__attribute__ ((noreturn)) void ptp_panic(char *fmt, ...);

// 1mb default buffer size
#define CAMLIB_DEFAULT_SIZE 1000000

/// @brief Camlib library errors, not PTP return codes
enum PtpGeneralError {
	PTP_OK = 0,
	PTP_NO_DEVICE = -1,
	PTP_NO_PERM = -2,
	PTP_OPEN_FAIL = -3,
	PTP_OUT_OF_MEM = -4,
	PTP_IO_ERR = -5,
	PTP_RUNTIME_ERR = -6,
	PTP_UNSUPPORTED = -7,
	PTP_CHECK_CODE = -8,
	PTP_CANCELED = -9,
};

/// @brief Evaluates PtpGeneralError into string message
const char *ptp_perror(int rc);

enum PtpLiveViewType {
	PTP_LV_NONE = 0,
	PTP_LV_EOS = 1,
	PTP_LV_CANON = 2,
	PTP_LV_ML = 3, // ptplv v1
	PTP_LV_EOS_ML_BMP = 4, // ptplv v2
};

/// @brief Unique camera types - each type should have similar opcodes and behavior
enum PtpVendors {
	PTP_DEV_EMPTY = 0,
	PTP_DEV_EOS = 1,
	PTP_DEV_CANON = 2,
	PTP_DEV_NIKON = 3,
	PTP_DEV_SONY = 4,
	PTP_DEV_FUJI = 5,
	PTP_DEV_PANASONIC = 6,
};

// Wrapper types for vendor specific capture modes
enum ImageFormats {
	IMG_FORMAT_ETC = 0,
	IMG_FORMAT_RAW = 1,
	IMG_FORMAT_STD = 2,
	IMG_FORMAT_HIGH = 3,
	IMG_FORMAT_RAW_JPEG = 4,
};

/// @brief Tells lib what backend and packet style to use
enum PtpConnType {
	PTP_IP = (1 << 0),
	PTP_IP_USB = (1 << 1), // TCP-based, but using USB-style packets (Fujifilm)
	PTP_USB = (1 << 2),
};

/// @brief Linked list to handle currently possible values for a property
struct PtpPropAvail {
	void *next;
	void *prev;
	int code;
	int memb_size;
	int memb_cnt;
	void *data;
};

/// @brief Holds all camlib instance info
/// @struct PtpRuntime
struct PtpRuntime {
	/// @brief Set to 1 to kill all IO operations. By default, this is 1. When a valid connection
	/// is achieved by libusb, libwpd, and tcp backends, it will be set to 0. On IO error, it
	/// will be set to 1.
	uint8_t io_kill_switch;

	/// @brief One of enum PtpConnType
	/// @note Is set to USB by default
	uint8_t connection_type;

	/// @note The transaction ID and session ID is managed by the packet generator functions
	int transaction;
	int session;

	/// @brief Global buffer for data reading and writing
	/// @note This is volatile - it will grow in size (pointer will change) when needed.
    uint8_t *data;
    int data_length;

	/// @note For optimization on libusb, as many bytes as possible should be read at once. Generally this
	/// is 512, but certain comm backends can manage more. For TCP, this isn't used.
	int max_packet_size;

	/// @brief Info about current connection, used to detect camera type, supported opodes, etc
	/// @note Set by ptp_parse_device_info.
	struct PtpDeviceInfo *di;
	int device_type;

	/// @brief For Windows compatibility, this is set to indicate lenth for a data packet
	/// that will be sent after a command packet. Will be set to zero when ptp_send_bulk_packets is called.
	int data_phase_length;

	/// @brief For session comm/io structures (holds backend instance pointers)
	void *comm_backend;

	void *userdata;

	/// @brief Optional (see CAMLIB_DONT_USE_MUTEX)
	pthread_mutex_t *mutex;

	/// @brief Optionally wait up to 256 seconds for a response. Some PTP operations require this, such as EOS capture.
	/// @note Not thread safe. Will be reset after each operation.
	uint8_t wait_for_response;

	/// @brief Default value for wait_for_response.
	uint8_t response_wait_default;

	/// @brief For devices that implement it, this will hold a linked list of properties and an array of their supported values.
	/// generic_ functions will reject set property calls if an invalid value is written.
	/// @note: Optional
	struct PtpPropAvail *avail;

	struct ObjectCache *oc;
};

/// @brief Generic event / property change
struct PtpGenericEvent {
	uint16_t code;
	const char *name;
	int value;
	const char *str_value;
};

/// @brief Generic PTP command structure - accepted by operation API
struct PtpCommand {
	uint16_t code;
	uint32_t params[5];
	int param_length;
	int data_length;
};

/// @brief Generic Struct for arrays
struct PtpArray {
	uint32_t length;
	uint32_t data[];
};

/// @brief Returns the return code (RC) currently in the data buffer.
/// @note Not thread safe.
/// @memberof PtpRuntime
DLL_EXPORT int ptp_get_return_code(struct PtpRuntime *r);

/// @brief Get number of parameters in packet in data buffer
/// @note Not thread safe.
/// @memberof PtpRuntime
DLL_EXPORT int ptp_get_param_length(struct PtpRuntime *r);

/// @brief Get parameter at index i
/// @note Not thread safe.
/// @memberof PtpRuntime
DLL_EXPORT uint32_t ptp_get_param(struct PtpRuntime *r, int i);

/// @brief Get transaction ID of packet in the data buffer
/// @note Not thread safe.
/// @memberof PtpRuntime
DLL_EXPORT int ptp_get_last_transaction_id(struct PtpRuntime *r);

/// @brief Get ptr of packet payload in data buffer, after packet header
/// @note Not thread safe.
/// @memberof PtpRuntime
DLL_EXPORT uint8_t *ptp_get_payload(struct PtpRuntime *r);

/// @brief Get length of payload returned by ptp_get_payload
/// @note Not thread safe.
/// @memberof PtpRuntime
DLL_EXPORT int ptp_get_payload_length(struct PtpRuntime *r);

/// @brief Allocate new PtpRuntime based on bitfield options - see PtpConnType
/// @memberof PtpRuntime
DLL_EXPORT struct PtpRuntime *ptp_new(int options);

/// @brief Reset all session-specific fields of PtpRuntime - both libusb and libwpd backends call
/// this before establishing connection, so calling this is not required
/// @memberof PtpRuntime
DLL_EXPORT void ptp_reset(struct PtpRuntime *r);

/// @brief Init PtpRuntime locally - uses default recommended settings (USB)
/// @memberof PtpRuntime
DLL_EXPORT void ptp_init(struct PtpRuntime *r);

/// @brief Frees PtpRuntime data buffer - doesn't free the actual structure, or device info (yet)
/// @memberof PtpRuntime
DLL_EXPORT void ptp_close(struct PtpRuntime *r);

/// @brief Send a command request to the device with no data phase
/// @memberof PtpRuntime
DLL_EXPORT int ptp_send(struct PtpRuntime *r, struct PtpCommand *cmd);

/// @brief Send a command request to the device with a data phase (thread safe)
/// @memberof PtpRuntime
DLL_EXPORT int ptp_send_data(struct PtpRuntime *r, struct PtpCommand *cmd, void *data, int length);

/// @brief Try and get an event from the camera over int endpoint (USB-only)
/// @memberof PtpRuntime
DLL_EXPORT int ptp_get_event(struct PtpRuntime *r, struct PtpEventContainer *ec);

/// @brief Unlock the IO mutex (unless it was kept locked)
/// @memberof PtpRuntime
DLL_EXPORT void ptp_mutex_unlock(struct PtpRuntime *r);

/// @brief Keep the mutex locked one more time for the current thread
/// @note  When calling a thread-safe function, this will garuntee the mutex locked, in the
/// case that you want to continue using the buffer. Must be unlocked or will cause deadlock.
/// @note camlib uses a recursive mutex.
/// @memberof PtpRuntime
DLL_EXPORT void ptp_mutex_keep_locked(struct PtpRuntime *r);

/// @brief Lock the IO mutex - only should be used by backend
/// @memberof PtpRuntime
DLL_EXPORT void ptp_mutex_lock(struct PtpRuntime *r);

/// @brief Gets type of device from r->di
/// @returns enum PtpDeviceType
/// @memberof PtpRuntime
DLL_EXPORT int ptp_device_type(struct PtpRuntime *r);

/// @brief Check if an opcode is supported by looking through supported props in r->di
/// @returns 1 if yes, 0 if no
/// @memberof PtpRuntime
DLL_EXPORT int ptp_check_opcode(struct PtpRuntime *r, int opcode);

/// @brief Check if a property code is supported by looking through supported props in r->di
/// @returns 1 if yes, 0 if no
/// @memberof PtpRuntime
DLL_EXPORT int ptp_check_prop(struct PtpRuntime *r, int code);

/// @brief Mostly for internal use - realloc the data buffer
/// @note r->data will be reassigned, any old references must be updated
/// @memberof PtpRuntime
DLL_EXPORT int ptp_buffer_resize(struct PtpRuntime *r, size_t size);

DLL_EXPORT int ptp_write_unicode_string(char *dat, char *string);
DLL_EXPORT int ptp_read_unicode_string(char *buffer, char *dat, int max);
DLL_EXPORT int ptp_read_utf8_string(void *dat, char *string, int max);
DLL_EXPORT int ptp_read_string(uint8_t *dat, char *string, int max);
DLL_EXPORT int ptp_write_string(uint8_t *dat, char *string);
DLL_EXPORT int ptp_write_utf8_string(void *dat, char *string);
DLL_EXPORT int ptp_read_uint16_array(uint8_t *dat, uint16_t *buf, int max, int *length);
DLL_EXPORT int ptp_read_uint16_array_s(uint8_t *bs, uint8_t *be, uint16_t *buf, int max, int *length);
inline static int ptp_write_u8 (void *buf, uint8_t out) { ((uint8_t *)buf)[0] = out; return 1; }
inline static int ptp_write_u16(void *buf, uint16_t out) { ((uint16_t *)buf)[0] = out; return 2; }
inline static int ptp_write_u32(void *buf, uint32_t out) { ((uint32_t *)buf)[0] = out; return 4; }
inline static int ptp_read_u32 (void *buf, uint32_t *out) { *out = ((uint32_t *)buf)[0]; return 4; }
inline static int ptp_read_u16 (void *buf, uint16_t *out) { *out = ((uint16_t *)buf)[0]; return 2; }
inline static int ptp_read_u8  (void *buf, uint8_t *out) { *out = ((uint8_t *)buf)[0]; return 1; }

// Build a new PTP/IP or PTP/USB command packet in r->data
DLL_EXPORT int ptp_new_cmd_packet(struct PtpRuntime *r, struct PtpCommand *cmd);

// Only for PTP_USB or PTP_USB_IP use
DLL_EXPORT int ptp_new_data_packet(struct PtpRuntime *r, struct PtpCommand *cmd, void *data, int data_length);

// Only use for PTP_IP
DLL_EXPORT int ptpip_data_start_packet(struct PtpRuntime *r, int data_length);
DLL_EXPORT int ptpip_data_end_packet(struct PtpRuntime *r, void *data, int data_length);

// Used only by ptp_open_session
DLL_EXPORT void ptp_update_transaction(struct PtpRuntime *r, int t);

// Set avail info for prop
DLL_EXPORT void ptp_set_prop_avail_info(struct PtpRuntime *r, int code, int memb_size, int cnt, void *data);

DLL_EXPORT void *ptp_dup_payload(struct PtpRuntime *r);

// Write r->data to a file called DUMP
/// @note Debugging only
DLL_EXPORT int ptp_dump(struct PtpRuntime *r);

#define CAMLIB_INCLUDE_IMPL
#include "cl_data.h"
#include "cl_backend.h"
#include "cl_ops.h"
#include "cl_enum.h"
#include "cl_bind.h"

// Backwards compatibility (mostly renamed functions)
#ifndef CAMLIB_NO_COMPAT
	#define ptp_get_last_transaction(...) ptp_get_last_transaction_id(__VA_ARGS__)
	#define ptp_generic_new(...) ptp_new(__VA_ARGS__)
	#define ptp_generic_close(...) ptp_close(__VA_ARGS__)
	#define ptp_generic_reset(...) ptp_reset(__VA_ARGS__)
	#define ptp_generic_init(...) ptp_init(__VA_ARGS__)
	#define ptp_generic_send(...) ptp_send(__VA_ARGS__)
	#define ptp_generic_send_data(...) ptp_send_data(__VA_ARGS__)
#endif

typedef void ptp_object_found_callback(struct PtpRuntime *r, struct PtpObjectInfo *oi, void *arg);

// Object service api (object.c) - optional
DLL_EXPORT struct ObjectCache *ptp_create_object_service(int *handles, int length, ptp_object_found_callback *callback, void *arg);
DLL_EXPORT struct PtpObjectInfo *ptp_object_service_get(struct PtpRuntime *r, struct ObjectCache *oc, int handle);
DLL_EXPORT struct PtpObjectInfo *ptp_object_service_get_index(struct PtpRuntime *r, struct ObjectCache *oc, int req_i);
DLL_EXPORT int ptp_object_service_length(struct PtpRuntime *r, struct ObjectCache *oc);
DLL_EXPORT int ptp_object_service_step(struct PtpRuntime *r, struct ObjectCache *oc);
DLL_EXPORT void ptp_object_service_add_priority(struct PtpRuntime *r, struct ObjectCache *oc, int handle);

#endif
