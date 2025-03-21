// Example implementation of function to safely kill the connection in a multithreaded application.
// Copyright 2024 by Daniel C (https://github.com/petabyt/libpict)

#include <libpict.h>

void ptp_report_error(struct PtpRuntime *r, char *reason, int code) {
	if (r->io_kill_switch) return;
	// Prevent any further operations if this is intentional
	if (code != 0) {
		r->operation_kill_switch = 1;
	}
	// Wait until we have a lock
	ptp_mutex_lock(r);
	// If io_kill_switch was set while waiting for a lock
	if (r->io_kill_switch) {
		ptp_mutex_unlock(r);
		return;
	}

	// Safely disconnect if intentional
	if (code == 0) {
		ptp_verbose_log("Closing session\n");
		ptp_close_session(r);
	}

	r->operation_kill_switch = 1;
	r->io_kill_switch = 1;

	ptp_mutex_unlock(r);

	if (reason == NULL) {
		if (code == PTP_IO_ERR) {
			ptp_verbose_log("Disconnected: IO Error\n");
		} else {
			ptp_verbose_log("Disconnected: Runtime error\n");
		}
	} else {
		ptp_verbose_log("Disconnected: %s\n", reason);
	}
}
