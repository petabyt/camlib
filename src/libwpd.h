// Header for libwpd
#ifndef PTP_WPD_H
#define PTP_WPD_H

#include <stdint.h>
#include <stddef.h>

// From Microsoft
typedef enum tagWPD_DEVICE_TYPES { 
	WPD_DEVICE_TYPE_GENERIC                       = 0,
	WPD_DEVICE_TYPE_CAMERA                        = 1,
	WPD_DEVICE_TYPE_MEDIA_PLAYER                  = 2,
	WPD_DEVICE_TYPE_PHONE                         = 3,
	WPD_DEVICE_TYPE_VIDEO                         = 4,
	WPD_DEVICE_TYPE_PERSONAL_INFORMATION_MANAGER  = 5,
	WPD_DEVICE_TYPE_AUDIO_RECORDER                = 6
}WPD_DEVICE_TYPES;

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

