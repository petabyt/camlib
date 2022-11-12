// Data packets - some very similar to the exact MTP/PTP spec,
// but can have variable size arrays in them, so it isn't compliant
#ifndef DATA_H
#define DATA_H

// Try and check for compatibility with 32 bit stuff
#include <stdint.h>
#if UINTPTR_MAX == 0xffffffff
#define BITS_32
#elif UINTPTR_MAX == 0xffffffffffffffff
#define BITS_64
#endif

// 4 Seems like a good limit?
struct PtpStorageIds {
	uint32_t length;
	uint32_t data[4];
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
	uint8_t read_only;
};

#endif
