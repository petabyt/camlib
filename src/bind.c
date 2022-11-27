// Generic text bindings to PTP functions
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <ptp.h>
#include <camlib.h>
#include <operations.h>
#include <backend.h>

struct BindResp {
	char *buffer;
	int params[10];
	int params_length;
};

struct RouteMap {
	char *name;
	int (*call)(struct BindResp *, struct PtpRuntime *);
};

const char *bind_error = "{\"error\": -3}";

int bind_init(struct BindResp *bind, struct PtpRuntime *r) {
	memset(r, sizeof(struct PtpRuntime), 0);
	r->data = malloc(CAMLIB_DEFAULT_SIZE);
	r->transaction = 0;
	r->session = 0;
	r->data_length = CAMLIB_DEFAULT_SIZE;
	r->di = NULL;
	return sprintf(bind->buffer, "{\"error\": %d}", 0);
}

int bind_connect(struct BindResp *bind, struct PtpRuntime *r) {
	// Check if uninitialized
	if (r->data_length != CAMLIB_DEFAULT_SIZE) {
		return sprintf(bind->buffer, "{\"error\": %d}", PTP_OUT_OF_MEM);
	}

	int x = ptp_device_init(r);
	return sprintf(bind->buffer, "{\"error\": %d}", x);
}

int bind_disconnect(struct BindResp *bind, struct PtpRuntime *r) {
	int x = ptp_device_close(r);
	return sprintf(bind->buffer, "{\"error\": %d}", x);
}

int bind_open_session(struct BindResp *bind, struct PtpRuntime *r) {
	int x = ptp_open_session(r);
	printf("open_session: %d\n", x);
	return sprintf(bind->buffer, "{\"error\": %d}", x);
}

int bind_close_session(struct BindResp *bind, struct PtpRuntime *r) {
	int x = ptp_close_session(r);
	return sprintf(bind->buffer, "{\"error\": %d}", x);
}

int bind_get_device_info(struct BindResp *bind, struct PtpRuntime *r) {
	if (r->di == NULL) {
		r->di = malloc(sizeof(struct PtpDeviceInfo));
	}

	int x = ptp_get_device_info(r, r->di);
	if (x) {
		return sprintf(bind->buffer, "{\"error\": %d}", x);
	}

	ptp_device_info_json(r->di, (char*)r->data, r->data_length);
	return sprintf(bind->buffer, "{\"error\": %d, \"resp\": %s}", x, (char*)r->data);
}

int bind_custom_cmd(struct BindResp *bind, struct PtpRuntime *r) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_CloseSession;
	cmd.param_length = bind->params_length - 1;
	for (int i = 0; i < cmd.param_length; i++) {
		cmd.params[i] = bind->params[i];
	}

	int x = ptp_generic_send(r, &cmd);
	return sprintf(bind->buffer, "{\"error\": %d, \"resp\": %X}", x, ptp_get_return_code(r));
}

int bind_drive_lens(struct BindResp *bind, struct PtpRuntime *r) {
	printf("Drive lens %d\n", bind->params[0]);
	int x = ptp_generic_drive_lens(r, bind->params[0]);
	return sprintf(bind->buffer, "{\"error\": %d}", x);
}

int bind_get_liveview_frame(struct BindResp *bind, struct PtpRuntime *r) {
	char *lv = malloc(ptp_liveview_size(r));
	int x = ptp_liveview_frame(r, lv);

	int err = x;
	if (x > 0) {
		err = 0;
	}

	char *inc = bind->buffer + sprintf(bind->buffer, "{\"error\": %d, \"resp\": [", err);

	for (int i = 0; i < x; i++) {
		if (i > x - 2) {
			inc += sprintf(inc, "%u", (uint8_t)lv[i]);
		} else {
			inc += sprintf(inc, "%u,", (uint8_t)lv[i]);
		}
	}

	inc += sprintf(inc, "]}");

	free(lv);
	return inc - bind->buffer;
}

int bind_get_liveview_type(struct BindResp *bind, struct PtpRuntime *r) {
	return sprintf(bind->buffer, "{\"error\": %d, \"resp\": %d}", 0, ptp_liveview_type(r));
}

int bind_get_liveview_frame_jpg(struct BindResp *bind, struct PtpRuntime *r) {
	int x = ptp_liveview_frame(r, bind->buffer);
	// TODO...
}

int bind_liveview_init(struct BindResp *bind, struct PtpRuntime *r) {
	return sprintf(bind->buffer, "{\"error\": %d, \"resp\": %d}", 0, ptp_liveview_type(r));
}

struct RouteMap routes[] = {
	{"ptp_connect", bind_connect},
	{"ptp_disconnect", bind_disconnect},
	{"ptp_init", bind_init},
	{"ptp_open_session", bind_open_session},
	{"ptp_close_session", bind_close_session},
	{"ptp_get_device_info", bind_get_device_info},
	{"ptp_drive_lens", bind_drive_lens},
	{"ptp_get_liveview_frame", bind_get_liveview_frame},
	{"ptp_get_liveview_type", bind_get_liveview_type},
	{"ptp_get_liveview_frame.jpg", bind_get_liveview_frame_jpg},
	{"ptp_liveview_init", bind_liveview_init},
};

// Returns -1 for error - return length of content otherwise
int bind_run(struct PtpRuntime *r, char *req, char *buffer) {
	if (buffer == NULL) {
		return -1;
	}

	struct BindResp bind;
	bind.params_length = 0;
	bind.buffer = buffer;

	for (int i = 0; req[i] != '\0'; i++) {
		if (req[i] == ',') {
			req[i] = '\0'; // tear off for strcmp
			i++;
			char *base = req + i;
			while (*base != ';') {
				bind.params[bind.params_length] = strtol(base, &base, 10);
				bind.params_length++;
				if (*base != ',') break;
				base++;
			}

			break;
		}
	}

	for (int i = 0; i < (int)(sizeof(routes) / sizeof(struct RouteMap)); i++) {
		if (!strcmp(routes[i].name, req)) {
			return routes[i].call(&bind, r);
		}
	}

	return -1;
}
