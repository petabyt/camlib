#ifndef BIND_H
#define BIND_H

// Recommended buffer size for bind_run
#define PTP_BIND_DEFAULT_SIZE 5000000

#define BIND_MAX_PARAM 5
#define BIND_MAX_NAME 64
#define BIND_MAX_STRING 32
#define BIND_MAX_BYTES 128

struct BindReq {
	char *buffer;
	int max;
	char name[BIND_MAX_NAME];

	int params[BIND_MAX_PARAM];
	int params_length;

	char string[BIND_MAX_STRING];

	// TODO: Implement this
	uint8_t bytes[BIND_MAX_BYTES];
};

// Run a binding - will return JSON
int bind_run(struct PtpRuntime *r, char *req, char *buffer, int size);

// Run a binding directly from the structure
int bind_run_req(struct PtpRuntime *r, struct BindReq *bind, char *buffer, int max);

#endif
