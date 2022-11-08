#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <camlib.h>
#include <backend.h>
#include <ptp.h>
#include <operations.h>

#include <elk.h>

struct PtpRuntime global_runtime;

static jsval_t js_print(struct js *js, jsval_t *args, int nargs) {
	for (int i = 0; i < nargs; i++) {
		const char *space = i == 0 ? "" : " ";
		printf("%s%s", space, js_str(js, args[i]));
	}

	putchar('\n');  // Finish by newline
	return js_mkundef();
}

static jsval_t js_ptp_device_init(struct js *js, jsval_t *args, int nargs) {
	global_runtime.transaction = 0;
	global_runtime.session = 0;
	return js_mknum(ptp_device_init(&global_runtime));
}

static jsval_t js_ptp_device_close(struct js *js, jsval_t *args, int nargs) {
	return js_mknum(ptp_device_close(&global_runtime));
}

static jsval_t js_ptp_get_return_code(struct js *js, jsval_t *args, int nargs) {
	return js_mknum(ptp_get_return_code(&global_runtime));
}

static jsval_t js_ptp_device_info_json(struct js *js, jsval_t *args, int nargs) {
	struct PtpDeviceInfo di;
	int x = ptp_get_device_info(&global_runtime, &di);

	int length = ptp_device_info_json(&di, (char*)global_runtime.data, global_runtime.data_length);

	return js_mkstr(js, (char*)global_runtime.data, length);
}

#define SIZE 1000000
#define SIZE_JS 1000000
int main(int argc, char *argv[]) {
	global_runtime.data = malloc(SIZE);
	global_runtime.data_length = SIZE;

	if (argc == 1) {
		printf("No parameter\n");
		return 1;
	}

	FILE *f = fopen(argv[1], "r");
	if (f == NULL) {
		printf("File not found\n");
		return 1;
	}
	
	fseek(f, 0, SEEK_END);
	int length = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *text = malloc(length);
	fread(text, 1, length, f);
	fclose(f);

	char *mem = malloc(SIZE_JS);
	struct js *js = js_create(mem, SIZE_JS);

	js_set(js, js_glob(js), "print", js_mkfun(js_print));

	js_set(js, js_glob(js), "ptp_device_init", js_mkfun(js_ptp_device_init));
	js_set(js, js_glob(js), "ptp_get_return_code", js_mkfun(js_ptp_get_return_code));
	js_set(js, js_glob(js), "ptp_device_info_json", js_mkfun(js_ptp_device_info_json));
	js_set(js, js_glob(js), "ptp_ptp_device_close", js_mkfun(js_ptp_device_close));

	jsval_t v = js_eval(js, text, length);

	printf("result: %s\n", js_str(js, v));

	free(mem);
	free(text);
	return 0;
}

