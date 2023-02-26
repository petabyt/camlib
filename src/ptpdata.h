// Data packets - some very similar to the exact MTP/PTP spec,
// but can have variable size arrays in them, so it isn't compliant
#ifndef DATA_H
#define DATA_H

// Try and check for compatibility with 32 bit stuff
// uint64_t is only used once
#include <stdint.h>
#if UINTPTR_MAX == 0xffffffff
#define BITS_32
#elif UINTPTR_MAX == 0xffffffffffffffff
#define BITS_64
#endif

// Need to avoid structure packing - most architectures are fine with this
// (accessing a 32 bit integer at an unaligned address - but some might have problems)
#pragma pack(push, 1)

// 4 Seems like a good limit?
struct PtpStorageIds {
	uint32_t length;
	uint32_t data[4];
};

struct UintArray {
	uint32_t length;
	uint32_t data[];
};

// To store unpacked device info data, after parsing
struct PtpDeviceInfo {
	uint16_t standard_version;
	uint32_t vendor_ext_id;
	uint16_t version;
	char extensions[128];
	uint16_t functional_mode;

	int ops_supported_length;
	uint16_t ops_supported[256];

	int events_supported_length;
	uint16_t events_supported[128];

	int props_supported_length;
	uint16_t props_supported[64];

	int capture_formats_length;
	uint16_t capture_formats[16];

	int playback_formats_length;
	uint16_t playback_formats[16];

	char manufacturer[128];
	char model[128];
	char device_version[64];
	char serial_number[128];
};

struct PtpStorageInfo {
	uint16_t storage_type;
	uint16_t fs_type;
	uint16_t access_capability;
#ifdef BITS_32
	uint32_t max_capacity;
	uint32_t max_capacity_64;
	uint32_t free_space;
	uint32_t free_space_64;
#endif
#ifdef BITS_64
	uint64_t max_capacity;
	uint64_t free_space;
#endif
	uint32_t free_objects;
};

struct ObjectRequest {
	uint32_t storage_id; // left zero to get all
	uint32_t object_format;
	uint32_t object_handle;
	uint8_t all_storage_ids;
	uint8_t all_formats;
	uint8_t in_root;
};

struct PtpObjectInfo {
	uint32_t storage_id;
	uint16_t obj_format;
	uint16_t protection;
	uint32_t compressed_size;
	uint16_t thumb_size;
	uint32_t thumb_compressed_size;
	uint32_t thumb_width;
	uint32_t thumb_height;
	uint32_t img_width;
	uint32_t img_height;
	uint32_t img_bit_depth;
	uint32_t parent_obj;
	uint16_t assoc_type; // association
	uint32_t assoc_desc;
	uint32_t sequence_num;

	#define PTP_OBJ_INFO_VAR_START 52

	char filename[64];
	char date_created[32];
	char date_modified[32];
	char keywords[64];
};

struct PtpDevPropDesc {
	uint16_t code;
	uint16_t data_type;
	uint8_t read_only; // (get/set)

	#define PTP_PROP_DESC_VAR_START 5

	int default_value;
	int current_value;
};

struct PtpObjPropDesc {
	uint32_t property_code;
	uint32_t data_type;
	uint8_t get_set;
	uint32_t default_value;
	uint32_t group_code;
	uint32_t form_flag;
	// mystery data type follows if form_flag == 0
};

struct PtpCanonEvent {
	int type;
	int code;
	int value;
	int def;
};

struct PtpEOSViewFinderData {
	uint32_t length;
	uint32_t type;
	// standard JPG follows
};

struct PtpEOSObject {
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t d;
	uint32_t e;
};

int ptp_parse_device_info(struct PtpRuntime *r, struct PtpDeviceInfo *di);
int ptp_device_info_json(struct PtpDeviceInfo *di, char *buffer, int max);
int ptp_parse_prop_desc(struct PtpRuntime *r, struct PtpDevPropDesc *oi);
int ptp_parse_object_info(struct PtpRuntime *r, struct PtpObjectInfo *oi);
int ptp_pack_object_info(struct PtpRuntime *r, struct PtpObjectInfo *oi);
int ptp_storage_info_json(struct PtpStorageInfo *so, char *buffer, int max);
int ptp_object_info_json(struct PtpObjectInfo *so, char *buffer, int max);

void *ptp_open_eos_events(struct PtpRuntime *r);
void *ptp_get_eos_event(struct PtpRuntime *r, void *e, struct PtpCanonEvent *ce);

int ptp_eos_events_json(struct PtpRuntime *r, char *buffer, int max);

int ptp_eos_get_shutter(int data, int dir);
int ptp_eos_get_iso(int data, int dir);
int ptp_eos_get_aperture(int data, int dir);
int ptp_eos_get_white_balance(int data, int dir);
int *ptp_eos_get_imgformat_data(int code);
int ptp_eos_get_imgformat_value(int data[5]);

#pragma pack(pop)

#endif
