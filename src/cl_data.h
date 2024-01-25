// PTP data structures - some very similar to the exact MTP/PTP spec.
// Variable sized arrays/strings are replaced with fixed arrays, to allow
// packing and unpacking in the same struct.
#ifndef CL_DATA_H
#define CL_DATA_H

// Need to avoid structure packing - most architectures are fine with this
// (accessing a 32 bit integer at an unaligned address - but some might have problems)
#pragma pack(push, 1)

// 4 Seems like a good limit?
struct PtpStorageIds {
	uint32_t length;
	uint32_t data[4];
};

// TODO: Rename PtpUintArray
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
	uint16_t props_supported[128];

	int capture_formats_length;
	uint16_t capture_formats[32];

	int playback_formats_length;
	uint16_t playback_formats[32];

	char manufacturer[128];
	char model[128];
	char device_version[64];
	char serial_number[128];
};

struct PtpStorageInfo {
	uint16_t storage_type;
	uint16_t fs_type;
	uint16_t access_capability;
	uint64_t max_capacity;
	uint64_t free_space;
	uint32_t free_objects;
};

struct PtpObjectInfo {
	uint32_t storage_id;
	uint16_t obj_format;
	uint16_t protection;
	uint32_t compressed_size;
	uint16_t thumb_format;
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

struct PtpMlLvInfo {
	uint32_t lv_pitch;
	uint32_t lv_width;
	uint32_t lcd_palette[256];
};

struct PtpMlLvHeader {
	uint8_t digic;
	uint8_t v1;
	uint8_t v2;
	uint8_t v3;
};

enum PtpMlBmpLvOption {
	PTP_ML_BMP_LV_GET_FRAME = 0,
	PTP_ML_BMP_LV_GET_SPEC = 1,
};

enum PtpCHDKCommands {
	PTP_CHDK_Version = 0,
	PTP_CHDK_UploadFile = 5,
};

// Response to struct FujiInitPacket
struct PtpFujiInitResp {
	uint32_t x1;
	uint32_t x2;
	uint32_t x3;
	uint32_t x4;
	char cam_name[54];
};

// Appears to be an array for events
struct PtpFujiEvents {
	uint16_t length;
	struct PtpFujiEventsEntry {
		uint16_t code;
		uint32_t value;
	}events[];
};

// Looks very similar to standard ISO ObjectInfo, but random bits moved around.
// variable data starts at same location.
struct PtpFujiObjectInfo {
	uint32_t storage_id;
	uint16_t obj_format;
	uint16_t protection;
	uint32_t fuji_max_partial_size;
	uint8_t fuji_unknown2;
	uint32_t compressed_size;
	uint16_t fuji_unknown3;
	uint16_t fuji_unknown4;
	uint8_t fuji_unknown5;
	uint32_t img_width;
	uint32_t img_height;
	uint32_t img_bit_depth;
	uint32_t parent_obj;
	uint16_t assoc_type;
	uint32_t assoc_desc;
	uint32_t sequence_num;

	#define PTP_FUJI_OBJ_INFO_VAR_START 52

	char filename[64];
	char date_created[32];
	char settings[32];
	char meta[32];
};

#pragma pack(pop)

#ifdef CAMLIB_INCLUDE_IMPL
int ptp_pack_object_info(struct PtpRuntime *r, struct PtpObjectInfo *oi, void **d, int max);

int ptp_parse_prop_value(struct PtpRuntime *r);
int ptp_parse_device_info(struct PtpRuntime *r, struct PtpDeviceInfo *di);
int ptp_device_info_json(struct PtpDeviceInfo *di, char *buffer, int max);
int ptp_parse_prop_desc(struct PtpRuntime *r, struct PtpDevPropDesc *oi);
int ptp_parse_object_info(struct PtpRuntime *r, struct PtpObjectInfo *oi);
int ptp_storage_info_json(struct PtpStorageInfo *so, char *buffer, int max);
int ptp_object_info_json(struct PtpObjectInfo *so, char *buffer, int max);

int ptp_eos_events(struct PtpRuntime *r, struct PtpGenericEvent **p);
void *ptp_open_eos_events(struct PtpRuntime *r);
void *ptp_get_eos_event(struct PtpRuntime *r, void *e, struct PtpCanonEvent *ce);

int ptp_eos_events_json(struct PtpRuntime *r, char *buffer, int max);

// Standard property value converters (conv.c)
int ptp_eos_get_shutter(int data, int dir);
int ptp_eos_get_iso(int data, int dir);
int ptp_eos_get_aperture(int data, int dir);
int ptp_eos_get_white_balance(int data, int dir);
int ptp_eos_get_imgformat_value(int data[5]);

void *ptp_pack_chdk_upload_file(struct PtpRuntime *r, char *in, char *out, int *length);

// Fuji (PTP/IP)
int ptp_fuji_get_init_info(struct PtpRuntime *r, struct PtpFujiInitResp *resp);
int ptp_fuji_parse_object_info(struct PtpRuntime *r, struct PtpFujiObjectInfo *oi);

#endif

#endif
