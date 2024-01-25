// Advanced developer opcodes for Canon EOS cameras - this is not meant to be compiled into
// camlib in most cases - but this file is available as an extension if you need it
// These commands can easily brick your camera (you can literally delete the firmware). Be careful please.
// Copyright 2023 by Daniel C (https://github.com/petabyt/camlib)

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <camlib.h>

#define EOS_TOK_INT 2
#define EOS_TOK_STR 4

struct EvProcParam {
	uint32_t type;
	uint32_t number;
	uint32_t p3;
	uint32_t p4;
	uint32_t size;
};

enum Types {
	TOK_TEXT,
	TOK_STR,
	TOK_INT,
};

#define MAX_STR 128
#define MAX_TOK 10

struct Tokens {
	struct T {
		int type;
		char string[MAX_STR];
		int integer;
	} t[MAX_TOK];
	int length;
};

// Required on some newer cameras, like the EOS M.
int ptp_eos_activate_command(struct PtpRuntime *r) {
	if (!ptp_check_opcode(r, PTP_OC_EOS_EnableEventProc)) {
		return 0;
	}

	for (int i = 0; i < 3; i++) {
		struct PtpCommand cmd;
		cmd.code = PTP_OC_EOS_EnableEventProc;
		cmd.param_length = 0;

		int ret = ptp_send(r, &cmd);
		if (ret == PTP_IO_ERR) {
			return ret;
		}
	}

	return 0;
}

int ptp_eos_exec_evproc(struct PtpRuntime *r, void *data, int length, int expect_return) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_EOS_ExecuteEventProc;
	cmd.param_length = 2;

	cmd.params[0] = 0; // async

	// If 1, evproc struct parser will put 3 params before params specified by our code
	cmd.params[1] = expect_return;

	return ptp_send_data(r, &cmd, data, length);
}

int ptp_eos_evproc_return_data(struct PtpRuntime *r) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_EOS_GetEventProcReturnData;
	cmd.param_length = 3;

	cmd.params[0] = 0;
	cmd.params[1] = 0;
	cmd.params[2] = 1;

	return ptp_send(r, &cmd);
}

static int alpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static int digit(char c) {
	return (c >= '0' && c <= '9');
}

static int hex(char c) {
	return digit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

// Parse a formatted command into struct Tokens
static struct Tokens *lex_evproc_command(char string[]) {
	struct Tokens *toks = malloc(sizeof(struct Tokens));
	int t = 0;
	memset(toks, 0, sizeof(struct Tokens));

	int c = 0;
	while (string[c] != '\0') {
		// Skip whitespace chars
		while (string[c] == ' ' || string[c] == '\t') {
			c++;
		}

		if (alpha(string[c])) {
			int s = 0;
			toks->t[t].type = TOK_TEXT;
			while (alpha(string[c])) {
				toks->t[t].string[s] = string[c];
				c++;
				s++;

				if (s > MAX_STR) {
					break;
				}
			}
			toks->t[t].string[s] = '\0';
		} else if (string[c] == '0' && string[c + 1] == 'x') {
			// Crappy hex parser, not sure about it yet, needs testing
			toks->t[t].integer = 0;
			toks->t[t].type = TOK_INT;
			c += 2;
			while (hex(string[c])) {
				toks->t[t].integer *= 16;
				if (string[c] >= '0' && string[c] <= '9') {
					toks->t[t].integer += string[c] - '0';
				}

				if (string[c] >= 'A' && string[c] <= 'F') {
					toks->t[t].integer += string[c] - 'A' + 10;
				}

				if (string[c] >= 'a' && string[c] <= 'f') {
					toks->t[t].integer += string[c] - 'a' + 10;
				}

				c++;
			}
		} else if (digit(string[c])) {
			toks->t[t].integer = 0;
			toks->t[t].type = TOK_INT;
			while (digit(string[c])) {
				toks->t[t].integer *= 10;
				toks->t[t].integer += string[c] - '0';
				c++;
			}
		} else if (string[c] == '\'') {
			c++;
			toks->t[t].type = TOK_STR;
			int s = 0;
			while (string[c] != '\'') {
				toks->t[t].string[s] = string[c];
				c++;
				s++;

				if (s > MAX_STR) {
					break;
				}
			}
			c++;
			toks->t[t].string[s] = '\0';
		} else {
			ptp_verbose_log("Skipping unknown character '%c'\n", string[c]);
			c++;
			continue;
		}

		if (t >= MAX_TOK) {
			ptp_verbose_log("Error: Hit max parameter count.\n");
			return NULL;
		} else {
			t++;
		}
	}

	toks->length = t;

	return toks;
}

char *canon_evproc_pack(int *length, char *string) {
	// Allocate some memory for the footer, we will use this later
	void *footer = malloc(500);
	void *footer_ptr = footer;
	uint32_t *long_args = footer;
	int footer_length = 0;

	// Set long_args to zero
	footer_length += ptp_write_uint32(&footer_ptr, 0);

	struct Tokens *toks = lex_evproc_command(string);

	// You will need to make sure your request never exceeds this
	// I can't be bothered to add bound checks
	char *data = malloc(500);

	if (toks->length == 0) {
		ptp_verbose_log("Error, must have at least 1 parameter.\n");
		return NULL;
	}

	// Add in initial parameter
	if (toks->t[0].type == TOK_TEXT) {
		int len = strlen(toks->t[0].string);
		memcpy(data, toks->t[0].string, len);
		data[len] = '\0';
		(*length) += len + 1;
	} else {
		ptp_verbose_log("Error, first parameter must be plain text.\n");
		return NULL;
	}

	// First uint32 is the number of parameters, we will save this ptr and modify it as we parse
	uint32_t *num_args = (uint32_t *)(data + (*length));
	(*num_args) = 0;
	(*length) += 4;

	// Pack parameters into data
	for (int t = 1; t < toks->length; t++) {
		switch (toks->t[t].type) {
		case TOK_INT: {
			struct EvProcParam integer;
			memset(&integer, 0, sizeof(struct EvProcParam));
			integer.type = EOS_TOK_INT;
			integer.number = toks->t[t].integer;

			memcpy(data + (*length), &integer, sizeof(struct EvProcParam));
			(*length) += sizeof(struct EvProcParam);

			(*num_args)++;
		} break;
		case TOK_STR: {
			struct EvProcParam pstring;
			memset(&pstring, 0, sizeof(struct EvProcParam));

			pstring.type = EOS_TOK_STR;
			pstring.size = strlen(toks->t[t].string) + 1;

			memcpy(data + (*length), &pstring, sizeof(struct EvProcParam));

			(*length) += sizeof(struct EvProcParam);

			footer_length += ptp_write_uint32((void **)&footer_ptr, 0);
			footer_length += ptp_write_utf8_string((void **)&footer_ptr, toks->t[t].string);
			(*long_args)++;

			(*num_args)++;
		} break;
		}
	}

	if (footer_length) {
		memcpy(data + (*length), footer, footer_length);
		(*length) += footer_length;
	}

	free(footer);
	free(toks);

	return data;	
}

static int eos_evproc(struct PtpRuntime *r, char *request, int payload) {
	int rc = ptp_eos_activate_command(r);
	if (rc) {
		ptp_verbose_log("Error activating command %d\n", rc);
		return rc;
	}

	int length = 0;
	void *data = canon_evproc_pack(&length, request);
	if (data == NULL) {
		return PTP_RUNTIME_ERR;
	}

	rc = ptp_eos_exec_evproc(r, data, length, payload);
	if (rc) {
		return rc;
	}

	free(data);

	return 0;
}

// This function is not optimized to be fast. Use it sparingly.
// canon_evproc_run(r, "FA_GetProperty %u %u", 0x1000008, 0);
int ptp_eos_evproc_run(struct PtpRuntime *r, char *fmt, ...) {
	char request[1024];
	va_list aptr;
	va_start(aptr, fmt);
	vsnprintf(request, sizeof(request), fmt, aptr);
	va_end(aptr);

	return eos_evproc(r, request, 0);
}

static int ptp_eos_evproc_run_payload(struct PtpRuntime *r, void **buf, char *fmt, ...) {
	char request[1024];
	va_list aptr;
	va_start(aptr, fmt);
	vsnprintf(request, sizeof(request), fmt, aptr);
	va_end(aptr);

	int rc = eos_evproc(r, request, 1);
	if (rc) return rc;

	rc = ptp_eos_evproc_return_data(r);

	void *dup = malloc(ptp_get_payload_length(r));
	memcpy(dup, ptp_get_payload(r), ptp_get_payload_length(r));

	(*buf) = dup;

	return rc;
}

int ptp_eos_fa_get_build_version(struct PtpRuntime *r, char *buffer, int max) {
	void *payload = NULL;

	int rc = ptp_eos_evproc_run_payload(r, &payload, "FA_GetProperty %d %d", 0x2000005, 0);

	strncpy(buffer, payload, max);

	return rc;
}
