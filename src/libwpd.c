// Windows USB I/O frontend for libwpd (https://github.com/petabyt/libwpd)
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>
#include <ptp.h>
#include <libwpd.h>

int ptp_comm_init(struct PtpRuntime *r) {
	ptp_reset(r);

	// We are not using low-level I/O operations, so this is never used
	r->max_packet_size = 512;

	// in libwpd, this will only coinitalize this thread.
	wpd_init(0, L"Camlib WPD");

	if (r->comm_backend != NULL) return 0;

	r->comm_backend = wpd_new();

	return 0;	
}

int ptp_device_init(struct PtpRuntime *r) {
	if (!r->io_kill_switch) {
		ptp_verbose_log("Connection is active\n");
		return PTP_IO_ERR;
	}

	ptp_comm_init(r);

	struct WpdStruct *wpd = (struct WpdStruct *)(r->comm_backend);
	if (wpd == NULL) return PTP_IO_ERR;

	ptp_mutex_lock(r);

	int length = 0;
	wchar_t **devices = wpd_get_devices(wpd, &length);

	if (length == 0) {
		ptp_mutex_unlock(r);
		return PTP_NO_DEVICE;
	}

	for (int i = 0; i < length; i++) {
		int ret = wpd_open_device(wpd, devices[i]);
		if (ret) {
			ptp_mutex_unlock(r);
			return PTP_OPEN_FAIL;
		}

		int type = wpd_get_device_type(wpd);
		ptp_verbose_log("Found device of type: %d\n", type);
		if (type == WPD_DEVICE_TYPE_CAMERA) {
			r->io_kill_switch = 0;
			r->operation_kill_switch = 0;
			ptp_mutex_unlock(r);
			return 0;
		}

		wpd_close_device(wpd);
	}

	ptp_mutex_unlock(r);
	return PTP_NO_DEVICE;
}

// Unimplemented, don't use
struct PtpDeviceEntry *ptpusb_device_list(struct PtpRuntime *r) {
	ptp_panic("Unsupported");
}

int ptp_device_open(struct PtpRuntime *r, struct PtpDeviceEntry *entry) {
	ptp_panic("Unsupported");
}

int ptp_cmd_write(struct PtpRuntime *r, void *to, int length) {
	if (r->io_kill_switch) return PTP_IO_ERR;
	struct WpdStruct *wpd = (struct WpdStruct *)(r->comm_backend);
	if (wpd == NULL) return PTP_IO_ERR;
	return wpd_ptp_cmd_write(wpd, to, length);
}

int ptp_cmd_read(struct PtpRuntime *r, void *to, int length) {
	if (r->io_kill_switch) return PTP_IO_ERR;
	struct WpdStruct *wpd = (struct WpdStruct *)(r->comm_backend);
	if (wpd == NULL) return PTP_IO_ERR;
	return wpd_ptp_cmd_read(wpd, to, length);
}

int ptp_read_int(struct PtpRuntime *r, void *to, int length) {
	return 0;
}

int ptp_device_close(struct PtpRuntime *r) {
	struct WpdStruct *wpd = (struct WpdStruct *)(r->comm_backend);
	if (wpd == NULL) return PTP_IO_ERR;
	wpd_close_device(wpd);
	return 0;
}

int ptp_device_reset(struct PtpRuntime *r) {
	return 0;
}
