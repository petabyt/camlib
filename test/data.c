#include <assert.h>
#include <string.h>
#include <camlib.h>
#include <stdint.h>

int test_data() {
	{
		uint8_t buffer[] = {05, 'H', 00, 'e', 00, 'l', 00, 'l', 00 ,'o', 00};
		char string[64];
		int rc = ptp_read_string(buffer, string, sizeof(string));
		assert(!strcmp(string, "Hello"));
		assert(rc == 11);
	}

	{
		uint8_t buffer[64];
		uint8_t ref[] = {05, 'H', 00, 'e', 00, 'l', 00, 'l', 00 ,'o', 00};
		int rc = ptp_write_string(buffer, "Hello");
		assert(!memcmp(buffer, ref, sizeof(ref)));
		assert(rc == 11);
	}

	return 0;
}
