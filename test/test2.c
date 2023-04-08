// ...
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>
#include <ptp.h>

int main() {
	struct PtpRuntime r;
	ptp_generic_init(&r);

	if (ptpip_device_init(&r)) {
		puts("Device connection error");
		return 0;
	}

	free(r.data);

	return 0;
}
