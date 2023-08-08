// Helper/convenient functions
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <camlib.h>
#include <ptp.h>

void ptp_generic_reset(struct PtpRuntime *r) {
	r->active_connection = 0;
	r->transaction = 0;
	r->session = 0;	
	r->max_packet_size = 512;
	r->data_phase_length = 0;
	r->di = NULL;
	r->connection_type = PTP_USB;
}

void ptp_generic_init(struct PtpRuntime *r) {
	ptp_generic_reset(r);
	r->data = malloc(CAMLIB_DEFAULT_SIZE);
	r->data_length = CAMLIB_DEFAULT_SIZE;

	r->mutex = malloc(sizeof(pthread_mutex_t));
	if (pthread_mutex_init(r->mutex, NULL)) {
		free(r->mutex);
		r->mutex = NULL;
	}
}

void ptp_mutex_lock(struct PtpRuntime *r) {
	pthread_mutex_lock(r->mutex);
}

void ptp_mutex_keep_locked(struct PtpRuntime *r) {
	r->caller_unlocks_mutex = 1;
}

void ptp_mutex_unlock(struct PtpRuntime *r) {
	pthread_mutex_unlock(r->mutex);
	r->caller_unlocks_mutex = 0;
}

void ptp_generic_close(struct PtpRuntime *r) {
	free(r->data);
}

struct UintArray * ptp_dup_uint_array(struct UintArray *arr) {
	struct UintArray *arr2 = malloc(4 + arr->length * 4);
	if (arr2 == NULL) return NULL;
	memcpy(arr2, arr, 4 + arr->length * 4);
	return arr2;
}

// May be slightly inneficient for every frame/action
// TODO: maybe 'cache' dev type for speed
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

// Perform a "generic" command type transaction. Could be a macro, but macros suck
int ptp_generic_send(struct PtpRuntime *r, struct PtpCommand *cmd) {
	pthread_mutex_lock(r);

	int length = ptp_new_cmd_packet(r, cmd);
	if (ptp_send_bulk_packets(r, length) != length) {
		pthread_mutex_unlock(r);
		return PTP_IO_ERR;
	}

	if (ptp_recieve_bulk_packets(r) < 0) {
		pthread_mutex_unlock(r);
		return PTP_IO_ERR;
	}

	if (ptp_get_return_code(r) == PTP_RC_OK) {
		if (!r->caller_unlocks_mutex) pthread_mutex_unlock(r);
		return 0;
	} else {
		if (!r->caller_unlocks_mutex) pthread_mutex_unlock(r);
		return PTP_CHECK_CODE;
	}
}

// Send a cmd packet, then data packet
// Perform a generic operation with a data phase to the camera
int ptp_generic_send_data(struct PtpRuntime *r, struct PtpCommand *cmd, void *data, int length) {
	pthread_mutex_lock(r);
	int plength = ptp_new_cmd_packet(r, cmd);

	r->data_phase_length = length;
	if (ptp_send_bulk_packets(r, plength) != plength) {
		pthread_mutex_unlock(r);
		return PTP_IO_ERR;
	}

	// TODO: Put this functionality in packet.c?
	cmd->param_length = 0;

	plength = ptp_new_data_packet(r, cmd);

	if (plength + length > r->data_length) {
		ptp_verbose_log("ptp_generic_send_data: Not enough memory\n");
		pthread_mutex_unlock(r);
		return PTP_OUT_OF_MEM;
	}

	memcpy(ptp_get_payload(r), data, length);
	ptp_update_data_length(r, plength + length);

	if (ptp_send_bulk_packets(r, plength + length) != plength + length) {
		pthread_mutex_unlock(r);
		return PTP_IO_ERR;
	}

	if (ptp_recieve_bulk_packets(r) < 0) {
		pthread_mutex_unlock(r);
		return PTP_IO_ERR;
	}

	if (ptp_get_return_code(r) == PTP_RC_OK) {
		if (!r->caller_unlocks_mutex) pthread_mutex_unlock(r);
		return 0;
	} else {
		if (!r->caller_unlocks_mutex) pthread_mutex_unlock(r);
		return PTP_CHECK_CODE;
	}
}

int ptp_dump(struct PtpRuntime *r) {
	FILE *f = fopen("DUMP", "w");
	fwrite(r->data, r->data_length, 1, f);
	fclose(f);
	return 0;
}
