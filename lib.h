#include <stdint.h>

// Generic command structure - order of data isn't important.
// Meant to be a bridge to the other packet types.
struct PtpCommand {
	int code;

	int params[5];
	int param_length;

	uint8_t *data;
	int data_length;
};
