// Canon and EOS Specific operation implementations
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>

#include <camlib.h>
#include <ptp.h>

int ptp_eos_remote_release_on(struct PtpRuntime *r, int mode) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_EOS_RemoteReleaseOn;
	cmd.param_length = 2;
	cmd.params[0] = mode;
	cmd.params[1] = 0;
	return ptp_generic_send(r, &cmd);
}

int ptp_eos_remote_release_off(struct PtpRuntime *r, int mode) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_EOS_RemoteReleaseOff;
	cmd.param_length = 1;
	cmd.params[0] = mode;
	return ptp_generic_send(r, &cmd);
}

int ptp_eos_cancel_af(struct PtpRuntime *r) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_EOS_AfCancel;
	cmd.param_length = 0;
	return ptp_generic_send(r, &cmd);
}

// Add 0x8000 to param1 change direction
int ptp_eos_drive_lens(struct PtpRuntime *r, int steps) {
	if (steps < 0) {
		steps = 0x8000 + (steps * -1);
	}
	
	struct PtpCommand cmd;
	cmd.code = PTP_OC_EOS_DriveLens;
	cmd.param_length = 1;
	cmd.params[0] = steps;
	return ptp_generic_send(r, &cmd);
}

int ptp_eos_set_event_mode(struct PtpRuntime *r, int mode) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_EOS_SetEventMode;
	cmd.param_length = 1;
	cmd.params[0] = mode;
	return ptp_generic_send(r, &cmd);
}

int ptp_eos_set_remote_mode(struct PtpRuntime *r, int mode) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_EOS_SetRemoteMode;
	cmd.param_length = 1;
	cmd.params[0] = mode;
	return ptp_generic_send(r, &cmd);
}

int ptp_eos_get_viewfinder_data(struct PtpRuntime *r) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_EOS_GetViewFinderData;
	cmd.param_length = 1;

	cmd.params[0] = 0x200000;
	cmd.params[1] = 0x0;
	cmd.params[2] = 0x0;

	return ptp_generic_send(r, &cmd);
}

int ptp_eos_get_prop_value(struct PtpRuntime *r, int code) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_EOS_GetDevicePropValue;
	cmd.param_length = 1;
	cmd.params[0] = code;
	return ptp_generic_send(r, &cmd);
}

int ptp_eos_set_prop_value(struct PtpRuntime *r, int code, int value) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_EOS_SetDevicePropValueEx;
	cmd.param_length = 0;

	uint32_t dat[] = {0xc, code, value};

	return ptp_generic_send_data(r, &cmd, dat, sizeof(dat));
}

// This is the same operation as ptp_eos_set_prop_value, but is more spiffy
int ptp_eos_set_prop_data(struct PtpRuntime *r, int code, void *data, int dlength) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_EOS_SetDevicePropValueEx;
	cmd.param_length = 0;

	uint32_t dat[] = {0x10 + dlength, code, ((uint32_t *)data)[0]};

	return ptp_generic_send_data(r, &cmd, dat, sizeof(dat));
}


int ptp_eos_get_event(struct PtpRuntime *r) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_EOS_GetEvent;
	cmd.param_length = 0;
	return ptp_generic_send(r, &cmd);	
}

int ptp_eos_ping(struct PtpRuntime *r) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_EOS_KeepDeviceOn;
	cmd.param_length = 0;
	return ptp_generic_send(r, &cmd);	
}

// The HDD capacity is pushed/popped (for lack of a better term)
// Not sure why, but his is how EOS Utility does it.
int ptp_eos_hdd_capacity_push(struct PtpRuntime *r) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_EOS_PCHDDCapacity;
	cmd.param_length = 3;
	cmd.params[0] = 0xfffffff7;
	cmd.params[1] = 0x1000;
	cmd.params[2] = 0x0;
	return ptp_generic_send(r, &cmd);
}

int ptp_eos_hdd_capacity_pop(struct PtpRuntime *r) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_EOS_PCHDDCapacity;
	cmd.param_length = 3;
	cmd.params[0] = 0x332d2d;
	cmd.params[1] = 0x1000;
	cmd.params[2] = 0x1;
	return ptp_generic_send(r, &cmd);
}

int ptp_eos_bulb_start(struct PtpRuntime *r) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_EOS_BulbStart;
	cmd.param_length = 0;
	return ptp_generic_send(r, &cmd);	
}

int ptp_eos_bulb_stop(struct PtpRuntime *r) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_EOS_BulbEnd;
	cmd.param_length = 0;
	return ptp_generic_send(r, &cmd);	
}

int ptp_eos_set_ui_lock(struct PtpRuntime *r) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_EOS_SetUILock;
	cmd.param_length = 1;
	cmd.params[0] = 0;
	return ptp_generic_send(r, &cmd);	
}

int ptp_eos_reset_ui_lock(struct PtpRuntime *r) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_EOS_ResetUILock;
	cmd.param_length = 0;
	return ptp_generic_send(r, &cmd);	
}

// Credit to lclevy/miniPtp for implementation
#define EOS_NAME_LEN 32
#define EOS_FIRM_MAX 0x200000
int ptp_eos_update_firmware(struct PtpRuntime *r, FILE *f, char *name) {
	if (strlen(name) > EOS_NAME_LEN) {
		return -1;
	}

	struct stat s;
	fstat(fileno(f), &s);
	printf("File size: %d\n", s.st_size);

	char *payload = malloc(EOS_FIRM_MAX);

	int sent = 0;
	while (1) {
		struct PtpCommand cmd;
		cmd.code = PTP_OC_EOS_UpdateFirmware;
		cmd.param_length = 2;
		cmd.params[0] = s.st_size;
		cmd.params[1] = sent;

		int size = EOS_FIRM_MAX;

		memset(payload, 0, EOS_NAME_LEN);
		strcpy(payload, name);
		fread(payload + EOS_NAME_LEN, size - EOS_NAME_LEN, 1, f);

		if (sent + size > s.st_size) size -= s.st_size - sent;

		int rc = ptp_generic_send_data(r, &cmd, payload, size);
		if (rc) {
			return rc;
		}

		printf("Sent %d\n", sent);

		sent += size - EOS_NAME_LEN;
		if (sent >= s.st_size) {
			return 0;
		}
	}
}
