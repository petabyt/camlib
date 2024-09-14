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

struct LibWPDPtpCommand {
	int code;
	uint32_t params[5];
	int param_length;
	int data_length;
};

/// @brief Initialize thread
int wpd_init(int verbose, wchar_t *app_name);

// Private, implemented by libwpd
struct WpdStruct {
	void *dev;
	void *results;
	void *values;
};

/// @brief Create new WpdStruct object
struct WpdStruct *wpd_new();

/// @brief Recieve an array of wide strings. Length is set in num_devices. Each string holds the device
/// ID, similar to Linux /dev/bus/usb/001
wchar_t **wpd_get_devices(struct WpdStruct *wpd, int *num_devices);

/// @brief Initialize the WpdStruct with a new connection to the selected device ID wide string
int wpd_open_device(struct WpdStruct *wpd, wchar_t *device_id);

/// @brief Close the device currently connected
int wpd_close_device(struct WpdStruct* wpd);

/// @brief Returns enum WPD_DEVICE_TYPES
int wpd_get_device_type(struct WpdStruct *wpd);

/// @brief Generic USB writing command. Will store all sent data into a buffer, which will be processed when wpd_ptp_cmd_read is called.
int wpd_ptp_cmd_write(struct WpdStruct *wpd, void *data, int size);

/// @brief Processes all PTP packets sent through wpd_ptp_cmd_write, and performs operations. Will then store all results in the 'out' buffer.
/// Every time this function is called, it will return data from the 'out' buffer, as requested by `size`.
int wpd_ptp_cmd_read(struct WpdStruct *wpd, void *data, int size);


// Manual old API:


/// @brief Send command packet with no data packet, but expect data packet (device may not send data packet, that is fine)
/// Data packet should be recieved with wpd_receive_do_data
int wpd_receive_do_command(struct WpdStruct *wpd, struct LibWPDPtpCommand *cmd);

/// @brief Get data payload from device, if any.
/// @note cmd struct will be filled with the response packet
int wpd_receive_do_data(struct WpdStruct *wpd, struct LibWPDPtpCommand *cmd, uint8_t *buffer, int length);

/// @brief Send command packet with data phase, with no data packet response from device. Call wpd_send_do_data to get the response 
/// and data packets
int wpd_send_do_command(struct WpdStruct* wpd, struct LibWPDPtpCommand* cmd, int length);

/// @brief Send the actual data packet, if wpd_send_do_command was called with length != 0
/// @note cmd struct will be filled with the response packet
int wpd_send_do_data(struct WpdStruct* wpd, struct LibWPDPtpCommand* cmd, uint8_t *buffer, int length);

#endif

