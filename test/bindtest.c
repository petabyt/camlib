#include <stdio.h>

#include <camlib.h>
#include <ptp.h>
#include <string.h>

// ptp_connect;
// ptp_drive_lens;-1;
// ptp_custom_send;4097,66,66,66,66,66;
// ptp_custom_cmd;4097,1,2,3,4,5
// ptp_set_property;"iso",6400

int main() {
	struct BindReq x;
	bind_parse(&x, "FOO_Bar;\"Hello, World\",12,-13;0,1,2,3,4,5,6,7,8,9;");
	printf("Name: %s\n", x.name);
	printf("String: %s\n", x.string);
	for (int i = 0; i < x.params_length; i++) {
		printf("Parameters: %d\n", x.params[i]);
	}
	for (int i = 0; i < x.bytes_length; i++) {
		printf("Bytes: %d\n", x.bytes[i]);
	}
}
