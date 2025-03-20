#include <assert.h>
#include <string.h>
#include <libpict.h>
#include <stdint.h>

int test_data(void) {
	{
		uint8_t buffer[] = {06, 'H', 00, 'e', 00, 'l', 00, 'l', 00 ,'o', 00, 00, 00};
		char string[64];
		int rc = ptp_read_string(buffer, string, sizeof(string));
		assert(rc == sizeof(buffer));
		assert(!strcmp(string, "Hello"));
	}

	{
		uint8_t buffer[64];
		uint8_t ref[] = {06, 'H', 00, 'e', 00, 'l', 00, 'l', 00 ,'o', 00, 00, 00};
		int rc = ptp_write_string(buffer, "Hello");
		assert(rc == 13);
		assert(!memcmp(buffer, ref, sizeof(ref)));
	}

	{
		uint8_t buffer[64];
		uint8_t ref[] = {0x0};
		int rc = ptp_write_string(buffer, "");
		assert(rc == 1);
		assert(!memcmp(buffer, ref, sizeof(ref)));	
	}

	return 0;
}
