#ifndef CL_BIND_H
#define CL_BIND_H

// Recommended buffer size for bind_run
#define PTP_BIND_DEFAULT_SIZE 5000000

#define BIND_MAX_PARAM 5
#define BIND_MAX_NAME 64
#define BIND_MAX_STRING 128
#define BIND_MAX_BYTES 512

struct BindReq {
	void *arg;
	int (*out)(struct BindReq *bind, char *fmt, ...);
	int (*out_bytes)(struct BindReq *bind, void *bytes, size_t length);

	char *buffer;
	int max;
	char name[BIND_MAX_NAME];

	int params[BIND_MAX_PARAM];
	int params_length;

	char *string;

	uint8_t *bytes;
	int bytes_length;
};

// Run a binding - will return JSON
//int bind_run(struct PtpRuntime *r, char *req, char *buffer, int size);

// Run a binding directly from the structure
int bind_run_req(struct PtpRuntime *r, struct BindReq *bind);

//void bind_parse(struct BindReq *br, char *req);

#endif
