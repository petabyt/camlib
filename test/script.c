#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include <camlib.h>
#include <ptpbackend.h>
#include <ptp.h>
#include <operations.h>

#include <mjs.h>

struct PtpRuntime global_runtime;

#define SIZE 1000000
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
	
	struct mjs *mjs = mjs_create();
	mjs_set_ffi_resolver(mjs, dlsym);
	mjs_exec(mjs, text, NULL);

	free(text);
	return 0;
}


