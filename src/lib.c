// Library frontend functions
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <camlib.h>
#include <ptp.h>

// Reset all fields of PtpRuntime - use this before reconnecting
void ptp_reset(struct PtpRuntime *r) {
	r->io_kill_switch = 1;
	r->transaction = 0;
	r->session = 0;	
	r->connection_type = PTP_USB;
	r->caller_unlocks_mutex = 0;
	r->wait_for_response = 1;
}

void ptp_init(struct PtpRuntime *r) {
	memset(r, 0, sizeof(struct PtpRuntime));
	ptp_reset(r);

	r->data = malloc(CAMLIB_DEFAULT_SIZE);
	r->data_length = CAMLIB_DEFAULT_SIZE;

	#ifndef CAMLIB_DONT_USE_MUTEX
	r->mutex = malloc(sizeof(pthread_mutex_t));
	if (pthread_mutex_init(r->mutex, NULL)) {
		ptp_verbose_log("Failed to init mutex\n");
		free(r->mutex);
		r->mutex = NULL;
	}
	#endif
}

struct PtpRuntime *ptp_new(int options) {
	struct PtpRuntime *r = malloc(sizeof(struct PtpRuntime));
	ptp_init(r);

	if (options & 3) {
		r->connection_type = options & 3;
	}

	return r;
}

int ptp_buffer_resize(struct PtpRuntime *r, size_t size) {
	// TODO: Adjust this in a clever way
	static int extra = 100;

	ptp_verbose_log("Extending IO buffer to %X\n", size + extra);
	r->data = realloc(r->data, size + extra);
	r->data_length = size + extra;
	if (r->data == NULL) {
		return PTP_OUT_OF_MEM;
	}

	return 0;
}

void ptp_mutex_lock(struct PtpRuntime *r) {
	if (r->mutex == NULL) return;
	pthread_mutex_lock(r->mutex);
}

void ptp_mutex_keep_locked(struct PtpRuntime *r) {
	r->caller_unlocks_mutex = 1;
}

void ptp_mutex_unlock(struct PtpRuntime *r) {
	if (r->mutex == NULL) return;
	pthread_mutex_unlock(r->mutex);
	r->caller_unlocks_mutex = 0;
}

void ptp_close(struct PtpRuntime *r) {
	free(r->data);
}

// Perform a generic command transaction - no data phase
int ptp_send(struct PtpRuntime *r, struct PtpCommand *cmd) {
	ptp_mutex_lock(r);

	r->data_phase_length = 0;

	int length = ptp_new_cmd_packet(r, cmd);
	if (ptp_send_bulk_packets(r, length) != length) {
		ptp_mutex_unlock(r);
		ptp_verbose_log("Didn't send all packets\n");
		return PTP_IO_ERR;
	}

	if (ptp_receive_bulk_packets(r) < 0) {
		ptp_mutex_unlock(r);
		ptp_verbose_log("Failed to recieve packets\n");
		return PTP_IO_ERR;
	}

	if (ptp_get_last_transaction_id(r) != r->transaction) {
		ptp_verbose_log("Mismatch transaction ID\n");
		//ptp_mutex_unlock(r);
		//return PTP_IO_ERR;
	}

	r->transaction++;

	if (ptp_get_return_code(r) == PTP_RC_OK) {
		if (!r->caller_unlocks_mutex) ptp_mutex_unlock(r);
		return 0;
	} else {
		ptp_verbose_log("Invalid return code: %X\n", ptp_get_return_code(r));
		if (!r->caller_unlocks_mutex) ptp_mutex_unlock(r);
		return PTP_CHECK_CODE;
	}
}

// Perform a command request with a data phase to the camera
int ptp_send_data(struct PtpRuntime *r, struct PtpCommand *cmd, void *data, int length) {
	ptp_mutex_lock(r);

	// Required for libWPD backend
	r->data_phase_length = length;

	// TODO: Expand buffer if needed
	// (Header packet will never be more than 100 bytes)
	if (length + 100 > r->data_length) {
		ptp_verbose_log("ptp_send_data: Not enough memory\n");
		ptp_mutex_unlock(r);
		return PTP_OUT_OF_MEM;
	}

	// Send operation request (data phase later on)
	int plength = ptp_new_cmd_packet(r, cmd);
	if (ptp_send_bulk_packets(r, plength) != plength) {
		ptp_mutex_unlock(r);
		return PTP_IO_ERR;
	}

	if (r->connection_type == PTP_IP) {
		// Send data start packet first (only has payload length)
		plength = ptpip_data_start_packet(r, length);
		if (ptp_send_bulk_packets(r, plength) != plength) {
			ptp_mutex_unlock(r);
			return PTP_IO_ERR;
		}

		// Send data end packet, with payload
		plength = ptpip_data_end_packet(r, data, length);
		if (ptp_send_bulk_packets(r, plength) != plength) {
			ptp_mutex_unlock(r);
			return PTP_IO_ERR;
		}
	} else {
		// Single data packet
		plength = ptp_new_data_packet(r, cmd, data, length);
		if (ptp_send_bulk_packets(r, plength) != plength) {
			ptp_mutex_unlock(r);
			ptp_verbose_log("Failed to send data packet (%d)\n", plength);
			return PTP_IO_ERR;
		}
	}

	if (ptp_receive_bulk_packets(r) < 0) {
		ptp_mutex_unlock(r);
		return PTP_IO_ERR;
	}

	if (ptp_get_last_transaction_id(r) != r->transaction) {
		ptp_verbose_log("ptp_send_data: Mismatch transaction ID (%d/%d)\n", ptp_get_last_transaction_id(r), r->transaction);
		//ptp_mutex_unlock(r);
		//return PTP_IO_ERR;
	}

	r->transaction++;

	if (ptp_get_return_code(r) == PTP_RC_OK) {
		if (!r->caller_unlocks_mutex) ptp_mutex_unlock(r);
		return 0;
	} else {
		if (!r->caller_unlocks_mutex) ptp_mutex_unlock(r);
		return PTP_CHECK_CODE;
	}
}

struct UintArray *ptp_dup_uint_array(struct UintArray *arr) {
	struct UintArray *arr2 = malloc(4 + arr->length * 4);
	if (arr2 == NULL) return NULL;
	memcpy(arr2, arr, 4 + arr->length * 4);
	return arr2;
}

int ptp_device_type(struct PtpRuntime *r) {
	struct PtpDeviceInfo *di = r->di;
	if (di == NULL) return PTP_DEV_EMPTY;
	if (!strcmp(di->manufacturer, "Canon Inc.")) {
		if (ptp_check_opcode(r, PTP_OC_EOS_GetStorageIDs)) {
			return PTP_DEV_EOS;
		}

		return PTP_DEV_CANON;
	} else if (!strcmp(di->manufacturer, "FUJIFILM")) {
		return PTP_DEV_FUJI;
	} else if (!strcmp(di->manufacturer, "Sony Corporation")) {
		return PTP_DEV_SONY;
	} else if (!strcmp(di->manufacturer, "Nikon Corporation")) {
		return PTP_DEV_NIKON;
	}

	return PTP_DEV_EMPTY;
}

int ptp_check_opcode(struct PtpRuntime *r, int op) {
	if (r->di == NULL) return 0;
	for (int i = 0; i < r->di->ops_supported_length; i++) {
		if (r->di->ops_supported[i] == op) {
			return 1;
		}
	}

	return 0;
}

int ptp_check_prop(struct PtpRuntime *r, int code) {
	if (r->di == NULL) return 0;
	for (int i = 0; i < r->di->props_supported_length; i++) {
		if (r->di->props_supported[i] == code) {
			return 1;
		}
	}

	return 0;
}

int ptp_dump(struct PtpRuntime *r) {
	FILE *f = fopen("DUMP", "w");
	fwrite(r->data, r->data_length, 1, f);
	fclose(f);
	return 0;
}
