#include <stdio.h>

#include <camlib.h>
#include <backend.h>
#include <ptp.h>
#include <operations.h>
#include <string.h>

// ptp_connect;
// ptp_drive_lens;-1;
// ptp_custom_send;4097,66,66,66,66,66;
// ptp_custom_cmd;4097,1,2,3,4,5
// ptp_set_property;"iso",6400

#define BIND_MAX_PARAM 5
#define BIND_MAX_NAME 64
#define BIND_MAX_STRING 32
struct BindResp {
	char *buffer;
	int max;
	char name[BIND_MAX_NAME];

	int params[BIND_MAX_PARAM];
	int params_length;

	char string[BIND_MAX_STRING];
};

void bind_parse(struct BindResp *br, char *req);

int main() {
	//struct PtpRuntime r;
	struct BindResp x;
	bind_parse(&x, "FOO_Bar;12");
	printf("Name: %s\n", x.name);
	printf("String: %s\n", x.string);
	printf("Parameters: %d\n", x.params[0]);
}
