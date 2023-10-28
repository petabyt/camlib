#include <camlib.h>

int ptp_pack_fuji_object_info(struct PtpRuntime *r, struct PtpObjectInfo *oi, void *buffer, int max) {
	void **ptr = (void **)(&buffer);

	memcpy(*ptr, oi, PTP_FUJI_OBJ_INFO_VAR_START);
	(*ptr) += PTP_FUJI_OBJ_INFO_VAR_START;
}

