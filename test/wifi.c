// Test basic opcode, get device properties
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>

int ptp_set_prop_value_16(struct PtpRuntime *r, int code, int value) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_SetDevicePropValue;
	cmd.param_length = 1;
	cmd.params[0] = code;

	uint16_t dat[] = {(uint16_t)value};

	return ptp_generic_send_data(r, &cmd, dat, sizeof(dat));
}

int main() {
	struct PtpRuntime r;
	ptp_generic_init(&r);
	r.connection_type = PTP_IP;
	if (ptpip_connect(&r, "192.168.0.1", FUJI_CMD_IP_PORT)) {
		puts("Device connection error");
		return 0;
	}

	if (ptpip_fuji_init(&r, "camlib")) {
		puts("Error on initialize");
	}

	// if (ptpip_connect_events(&r, "192.168.0.1", FUJI_CMD_IP_PORT)) {
		// puts("Device event connection error");
		// return 0;
	// }
// 
	// if (ptpip_init_events(&r)) {
		// puts("Error on events initialize");
		// return 0;
	// }

	struct PtpFujiInitResp resp;
	ptp_fuji_get_init_info(&r, &resp);

	printf("Connecting to %s\n", resp.cam_name);

	ptp_open_session(&r);

	ptpip_fuji_wait_unlocked(&r);

	ptp_set_prop_value(&r, PTP_PC_Fuji_Mode, 2); // set 16 bit
	ptp_set_prop_value(&r, PTP_PC_Fuji_TransferMode, 2); // set 32 bit

	struct UintArray *arr;
	int rc = ptp_get_storage_ids(&r, &arr);
	int id = arr->data[0];

	rc = ptp_get_object_handles(&r, id, PTP_OF_JPEG, 0, &arr);
	arr = ptp_dup_uint_array(arr);

	for (int i = 0; i < arr->length; i++) {
		ptp_set_prop_value(&r, PTP_PC_FUJI_Compression, 1);
		struct PtpObjectInfo oi;

		if (ptp_get_object_info(&r, arr->data[i], &oi)) {
			return 0;
		}

		printf("File size: %d\n", oi.compressed_size);
		printf("Filename: %s\n", oi.filename);

		ptp_get_prop_value(&r, PTP_PC_FUJI_PartialSize);

		if (ptp_download_file(&r, arr->data[i], oi.filename)) {
			return 0;
		}

		if (ptp_get_object_info(&r, arr->data[i], &oi)) {
			return 0;
		}


		ptp_set_prop_value(&r, PTP_PC_FUJI_Compression, 0);
		break;
	}

	free(arr);

	for (int i = 0; i < 100; i++) {
		puts("Pinging");
		if (ptp_fuji_ping(&r)) break;
		CAMLIB_SLEEP(1000);
	}

	// Camera shuts down after disconnect, would be nice to prevent that

	ptpip_close(&r);

	free(r.data);
	return 0;
}

