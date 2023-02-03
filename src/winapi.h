#ifndef PTP_WPD_H
#define PTP_WPD_H

#include <stdint.h>
#include <stddef.h>

// Headers to libwpd
int wpd_init(int verbose, wchar_t *app_name);

struct WpdStruct {
	void *dev;
	void *results;
	void *values;
};

wchar_t **wpd_get_devices(struct WpdStruct *wpd, int *num_devices);

int wpd_open_device(struct WpdStruct *wpd, wchar_t *device_id);
int wpd_close_device(struct WpdStruct* wpd);

int wpd_get_device_type(struct WpdStruct *wpd);

int wpd_recieve_do_command(struct WpdStruct *wpd, struct PtpCommand *cmd);

int wpd_recieve_do_data(struct WpdStruct *wpd, struct PtpCommand *cmd, uint8_t *buffer, int length);

int wpd_send_do_command(struct WpdStruct* wpd, struct PtpCommand* cmd, int length);

int wpd_send_do_data(struct WpdStruct* wpd, struct PtpCommand* cmd, uint8_t *buffer, int length);

#endif
