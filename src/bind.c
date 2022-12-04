// Generic text bindings to PTP functions
// The only function that is exposed is bind_run(), which returns
// valid JSON from a generic text like request
// This is not part of the core library, with will use malloc
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
	int max;
};

struct RouteMap {
	char *name;
	int (*call)(struct BindResp *, struct PtpRuntime *);
};

int bind_init(struct BindResp *bind, struct PtpRuntime *r) {
	memset(r, 0, sizeof(struct PtpRuntime));
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
	return snprintf(bind->buffer, bind->max, "{\"error\": %d, \"resp\": %s}", x, (char*)r->data);
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
		if (inc - bind->buffer >= i) {
			return 0;
		}

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

int bind_get_events(struct BindResp *bind, struct PtpRuntime *r) {
	int dev = ptp_detect_device(r);
	if (dev == PTP_DEV_EOS) {
		int x = ptp_eos_get_event(r);
		if (x) return sprintf(bind->buffer, "{\"error\": %d}", x);

		char *buffer = malloc(50000);
		ptp_eos_events_json(r, buffer, 50000);

		int len = snprintf(bind->buffer, bind->max, "{\"error\": %d, \"resp\": %s}", x, buffer);
		free(buffer);
		return len;
	}

	return sprintf(bind->buffer, "{\"error\": %d}", 0);
}

int bind_get_liveview_type(struct BindResp *bind, struct PtpRuntime *r) {
	return sprintf(bind->buffer, "{\"error\": %d, \"resp\": %d}", 0, ptp_liveview_type(r));
}

int bind_get_liveview_frame_jpg(struct BindResp *bind, struct PtpRuntime *r) {
	int x = ptp_liveview_frame(r, bind->buffer);

	if (x < 0) {
		return 0;
	}

	if (x > bind->max) {
		return 0;
	}

	memcpy(bind->buffer, bind->buffer + 8, x);

	return x;
}

int bind_liveview_init(struct BindResp *bind, struct PtpRuntime *r) {
	return sprintf(bind->buffer, "{\"error\": %d}", ptp_liveview_init(r));
}

int bind_detect_device(struct BindResp *bind, struct PtpRuntime *r) {
	return sprintf(bind->buffer, "{\"error\": %d, \"resp\": %d}", 0, ptp_detect_device(r));
}

int bind_eos_set_remote_mode(struct BindResp *bind, struct PtpRuntime *r) {
	return sprintf(bind->buffer, "{\"error\": %d}", ptp_eos_set_remote_mode(r, bind->params[0]));
}

int bind_eos_set_event_mode(struct BindResp *bind, struct PtpRuntime *r) {
	return sprintf(bind->buffer, "{\"error\": %d}", ptp_eos_set_event_mode(r, bind->params[0]));
}

int bind_hello_world(struct BindResp *bind, struct PtpRuntime *r) {
	return sprintf(bind->buffer, "COol beans");
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
	{"ptp_detect_device", bind_detect_device},
	{"ptp_get_events", bind_get_events},
//	{"ptp_custom_send", NULL},
//	{"ptp_custom_cmd", NULL},
	{"ptp_eos_set_remote_mode", bind_eos_set_remote_mode},
	{"ptp_eos_set_event_mode", bind_eos_set_event_mode},
	{"ptp_hello_world", bind_hello_world},
};

// See DOCS.md for documentation
int bind_run(struct PtpRuntime *r, char *req, char *buffer, int max) {
	if (buffer == NULL) {
		return -1;
	}

	struct BindResp bind;
	bind.params_length = 0;
	bind.buffer = buffer;
	bind.max = max;

	puts(req);

	for (int i = 0; req[i] != '\0'; i++) {
		if (req[i] == ';') {
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
