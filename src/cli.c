#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <libpict.h>

int ptp_decode_output(const char *mode, const char *input, const char *output);

struct Options {
	int do_open_sessions;	
};

static int usage(void) {
	printf(
		"camlib\n"
		"  --dec <input_file> <output_file> Decode any PTP/USB packet dump into a readable text file\n"
		"  --help\n"
		"  --run <operation> <args>\n"
		"  --dont-open-session (A session is opened/closed by default)\n"
		"  --run ptp_hello_world 1 2 3 \"Hello, World\"\n"
	);
	printf("Compile date: " __DATE__ "\n");
	return 0;
}

static int out_printf(struct BindReq *bind, char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
	return 0;
}

static int run_binding(struct Options *o, struct BindReq *br) {
	struct PtpRuntime *r = ptp_new(PTP_USB);

	if (ptp_device_init(r)) {
		printf("Device connection error\n");
		return 1;
	}

	ptp_open_session(r);

	int rc = bind_run_req(r, br);
	if (rc > 0) {
		printf("Command %s does not exist.\n", br->name);
	}
	if (rc) return rc;

	putchar('\n');

	ptp_close_session(r);

	ptp_device_close(r);
	ptp_close(r);
	return 0;
}

static int run_binding_yolo(struct Options *o, struct BindReq *br) {
	int rc = bind_run_req(NULL, br);
	return rc;
}

static int is_digit(char c) {return c >= '0' && c <= '9';}
static int parse_run(struct Options *o, int argc, char **argv, int i) {
	char *name = argv[i];

	i++;

	struct BindReq br;
	br.params_length = 0;
	br.bytes_length = 0;
	br.string = NULL;
	br.out = out_printf;
	br.out_bytes = NULL;

	memset(br.params, 0, sizeof(br.params));
	memset(br.name, 0, BIND_MAX_NAME);

	strcpy(br.name, name);

	for (; i < argc; i++) {
		if (is_digit(argv[i][0])) {
			br.params[br.params_length] = atoi(argv[i]);
			br.params_length++;
		}
	}

	int rc = run_binding(o, &br);
	return rc;
}

static int test(void) {
	struct PtpRuntime *r = ptp_new(PTP_USB);

	if (ptp_device_init(r)) {
		printf("Device connection error\n");
		return 1;
	}

	int rc = ptp_open_session(r);
	if (rc) return rc;

	struct PtpDeviceInfo di;

	char buffer[2048];
	rc = ptp_get_device_info(r, &di);
	if (rc) return rc;
	ptp_device_info_json(&di, buffer, sizeof(buffer));
	printf("%s\n", buffer);

	rc = ptp_eos_set_remote_mode(r, 1);
	if (rc) return rc;
	rc = ptp_eos_set_event_mode(r, 1);
	if (rc) return rc;

	for (int i = 0; i < 10; i++) {
		rc = ptp_eos_get_event(r);
		if (rc) return rc;
		usleep(10000);
	}

	rc = ptp_close_session(r);
	if (rc) return rc;

	ptp_device_close(r);
	ptp_close(r);
	return 0;
}

int main(int argc, char **argv) {
	extern int camlib_verbose;
	camlib_verbose = 1;
	struct Options o;
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--help")) {
			return usage();
		} else if (!strcmp(argv[i], "--run")) {
			return parse_run(&o, argc, argv, i + 1);
		} else if (!strcmp(argv[i], "--test")) {
			int rc = test();
			printf("Return code: %d\n", rc);
			return rc;
		} else if (!strcmp(argv[i], "--dec")) {
			char *type = "wifi";
			if ((argc - i) > 3) type = argv[i + 3];
			return ptp_decode_output(type, argv[i + 1], argv[i + 2]);
		}
	}

	return usage();
}
