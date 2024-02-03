// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)

// Generic text bindings to PTP functions
// - The only function that is exposed is bind_run(), which returns
// valid JSON from a generic text like request
// - This is not part of the core library, and will use malloc often

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <camlib.h>

#ifndef CAMLIB_VERSION
	#ifdef __DATE__
		#define CAMLIB_VERSION __DATE__
	#else
		#define CAMLIB_VERSION "Unknown"
	#endif
#endif

#ifndef CAMLIB_PLATFORM
	#ifdef WIN32
		#define CAMLIB_PLATFORM "windows"
	#else
		#define CAMLIB_PLATFORM "linux"
	#endif
#endif

// TODO: func to access these
int bind_connected = 0;
int bind_initialized = 0;

int bind_status(struct BindReq *bind, struct PtpRuntime *r) {
	return sprintf(bind->buffer, "{\"error\": 0, \"initialized\": %d, \"connected\": %d, "
	"\"platform\": \"%s\", \"version\": \"%s\"}",
		bind_initialized, bind_connected, CAMLIB_PLATFORM, CAMLIB_VERSION);
}

int bind_init(struct BindReq *bind, struct PtpRuntime *r) {
	if (bind_initialized) {
		ptp_close(r);
		if (r->di != NULL) free(r->di);
	}

	ptp_init(r);
	bind_initialized = 1;

	return sprintf(bind->buffer, "{\"error\": %d, \"buffer\": %d}", 0, r->data_length);
}

int bind_connect(struct BindReq *bind, struct PtpRuntime *r) {
	// Sanity check if uninitialized
	if (r->data_length != CAMLIB_DEFAULT_SIZE) {
		return sprintf(bind->buffer, "{\"error\": %d}", PTP_OUT_OF_MEM);
	}

	r->transaction = 0;
	r->session = 0;

	int x = ptp_device_init(r);
	if (!x) bind_connected = 1;
	return sprintf(bind->buffer, "{\"error\": %d}", x);
}

int bind_disconnect(struct BindReq *bind, struct PtpRuntime *r) {
	int x = ptp_device_close(r);
	if (!x) bind_connected = 0;
	return sprintf(bind->buffer, "{\"error\": %d}", x);
}

int bind_open_session(struct BindReq *bind, struct PtpRuntime *r) {
	int x = ptp_open_session(r);
	return sprintf(bind->buffer, "{\"error\": %d}", x);
}

int bind_close_session(struct BindReq *bind, struct PtpRuntime *r) {
	int x = ptp_close_session(r);
	return sprintf(bind->buffer, "{\"error\": %d}", x);
}

int bind_get_device_info(struct BindReq *bind, struct PtpRuntime *r) {
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

int bind_get_storage_ids(struct BindReq *bind, struct PtpRuntime *r) {
	struct PtpArray *arr;
	int x = ptp_get_storage_ids(r, &arr);
	if (x) return sprintf(bind->buffer, "{\"error\": %d}", x);

	int len = sprintf(bind->buffer, "{\"error\": %d, \"resp\": [", x);
	for (int i = 0; i < (int)arr->length; i++) {
		char *comma = "";
		if (i) comma = ",";
		len += sprintf(bind->buffer + len, "%s%u", comma, arr->data[i]);
	}

	len += sprintf(bind->buffer + len, "]}");
	return len;
}

int bind_get_storage_info(struct BindReq *bind, struct PtpRuntime *r) {
	struct PtpStorageInfo so;
	int x = ptp_get_storage_info(r, bind->params[0], &so);
	if (x) return sprintf(bind->buffer, "{\"error\": %d}", x);

	int len = sprintf(bind->buffer, "{\"error\": %d, \"resp\": ", x);
	len += ptp_storage_info_json(&so, bind->buffer + len, bind->max - len);
	len += sprintf(bind->buffer + len, "}");
	return len;
}

int bind_get_object_handles(struct BindReq *bind, struct PtpRuntime *r) {
	struct PtpArray *arr;
	// Parameters changed to correct order 14 nov 2023
	int x = ptp_get_object_handles(r, bind->params[0], bind->params[1], bind->params[2], &arr);
	if (x) return sprintf(bind->buffer, "{\"error\": %d}", x);

	int len = sprintf(bind->buffer, "{\"error\": %d, \"resp\": [", x);
	for (int i = 0; i < (int)arr->length; i++) {
		char *comma = "";
		if (i) comma = ",";
		len += sprintf(bind->buffer + len, "%s%u", comma, arr->data[i]);
	}

	len += sprintf(bind->buffer + len, "]}");
	return len;
}

int bind_get_object_info(struct BindReq *bind, struct PtpRuntime *r) {
	struct PtpObjectInfo oi;
	int x = ptp_get_object_info(r, bind->params[0], &oi);
	if (x) return sprintf(bind->buffer, "{\"error\": %d}", x);

	int len = sprintf(bind->buffer, "{\"error\": %d, \"resp\": ", x);

	len += ptp_object_info_json(&oi, bind->buffer + len, 1000);

	len += sprintf(bind->buffer + len, "}");
	return len;
}

int bind_custom(struct BindReq *bind, struct PtpRuntime *r) {
	struct PtpCommand cmd;
	cmd.code = bind->params[0];
	cmd.param_length = bind->params_length - 1;
	for (int i = 0; i < cmd.param_length; i++) {
		cmd.params[i] = bind->params[i + 1];
	}

	int x = 0;
	if (bind->bytes_length) {
		x = ptp_send_data(r, &cmd, bind->bytes, bind->bytes_length);
	} else {
		x = ptp_send(r, &cmd);
	}

	if (x) {
		return sprintf(bind->buffer, "{\"error\": %d}", x);
	}

	int len = sprintf(bind->buffer, "{\"error\": %d, \"resp\": %X, bytes: [", x, ptp_get_payload_length(r));
	for (int i = 0; i < ptp_get_payload_length(r); i++) {
		char *comma = "";
		if (i) comma = ",";
		len += sprintf(bind->buffer + len, "%s%u", comma, ptp_get_payload(r)[i]);
	}

	len += sprintf(bind->buffer + len, "]}");
	return len;
}

int bind_drive_lens(struct BindReq *bind, struct PtpRuntime *r) {
	int x;
	if (ptp_device_type(r) == PTP_DEV_EOS) {
		x = ptp_eos_drive_lens(r, bind->params[0]);
	} else {
		x = PTP_UNSUPPORTED;
	}

	return sprintf(bind->buffer, "{\"error\": %d}", x);
}

int bind_get_liveview_frame(struct BindReq *bind, struct PtpRuntime *r) {
	char *lv = malloc(ptp_liveview_size(r));
	int x = ptp_liveview_frame(r, lv);

	int err = x;
	if (x > 0) {
		err = 0;
	}

	char *inc = bind->buffer + sprintf(bind->buffer, "{\"error\": %d, \"resp\": [", err);

	for (int i = 0; i < x; i++) {
		if (inc - bind->buffer >= bind->max) {
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

int bind_ml_init_bmp_lv(struct BindReq *bind, struct PtpRuntime *r) {
	return sprintf(bind->buffer, "{\"error\": %d}",
		ptp_ml_init_bmp_lv(r)
	);
}

int bind_ml_get_bmp_lv(struct BindReq *bind, struct PtpRuntime *r) {
	uint32_t *buffer = 0;
	int x = ptp_ml_get_bmp_lv(r, &buffer);

	char *inc = bind->buffer + sprintf(bind->buffer, "{\"error\": %d, \"resp\": [", x);

	int size = 720 * 480;
	for (int i = 0; i < size; i++) {
		if (i > size - 2) {
			inc += sprintf(inc, "%u", buffer[i]);
		} else {
			inc += sprintf(inc, "%u,", buffer[i]);
		}
	}

	inc += sprintf(inc, "]}");

	free(buffer);
	return inc - bind->buffer;
}

int bind_set_property(struct BindReq *bind, struct PtpRuntime *r) {
	int dev = ptp_device_type(r);
	int x = 0;

	// Set a raw property value
	if (strlen(bind->string) == 0) {
		if (dev == PTP_DEV_EOS) {
			x = ptp_eos_set_prop_value(r, bind->params[0], bind->params[1]);
		} else {
			x = ptp_set_prop_value(r, bind->params[0], bind->params[1]);
		}
		return sprintf(bind->buffer, "{\"error\": %d}", x);
	}

	x = ptp_set_generic_property(r, bind->string, bind->params[0]);

	return sprintf(bind->buffer, "{\"error\": %d}", x);
}

int bind_get_events(struct BindReq *bind, struct PtpRuntime *r) {
	int dev = ptp_device_type(r);

	if (dev == PTP_DEV_EOS) {
		int x = ptp_eos_get_event(r);
		if (x) return sprintf(bind->buffer, "{\"error\": %d}", x);

		int len = snprintf(bind->buffer, bind->max, "{\"error\": 0, \"resp\": ");
		len += ptp_eos_events_json(r, bind->buffer + len, bind->max - len);
		
		len += snprintf(bind->buffer + len, bind->max - len, "}");

		return len;
	} else {
		struct PtpEventContainer ec;
		int x = ptp_get_event(r, &ec);
		if (x == 0) {
			// TODO: None of my devices get events
			return sprintf(bind->buffer, "{\"error\": 0, \"resp\": []}");
		} else if (x < 0) {
			return sprintf(bind->buffer, "{\"error\": %d}", x);
		}
	}

	return sprintf(bind->buffer, "{\"error\": %d}", 0);
}

int bind_get_all_props(struct BindReq *bind, struct PtpRuntime *r) {
	int dev = ptp_device_type(r);
	if (dev == PTP_DEV_EOS) {
		return bind_get_events(bind, r);
	} else {
		return sprintf(bind->buffer, "{\"error\": 0, \"resp\": []}");
		// TODO: loop through all camera devinfo properties
	}
}

int bind_get_liveview_type(struct BindReq *bind, struct PtpRuntime *r) {
	return sprintf(bind->buffer, "{\"error\": %d, \"resp\": %d}", 0, ptp_liveview_type(r));
}

int bind_get_liveview_frame_jpg(struct BindReq *bind, struct PtpRuntime *r) {
	int x = ptp_liveview_frame(r, bind->buffer);

	if (x < 0) {
		return 0;
	}

	if (x > bind->max) {
		return 0;
	}

	return x;
}

int bind_liveview_init(struct BindReq *bind, struct PtpRuntime *r) {
	return sprintf(bind->buffer, "{\"error\": %d}", ptp_liveview_init(r));
}

int bind_liveview_deinit(struct BindReq *bind, struct PtpRuntime *r) {
	return sprintf(bind->buffer, "{\"error\": %d}", ptp_liveview_deinit(r));
}

int bind_get_device_type(struct BindReq *bind, struct PtpRuntime *r) {
	return sprintf(bind->buffer, "{\"error\": %d, \"resp\": %d}", 0, ptp_device_type(r));
}

int bind_eos_set_remote_mode(struct BindReq *bind, struct PtpRuntime *r) {
	return sprintf(bind->buffer, "{\"error\": %d}", ptp_eos_set_remote_mode(r, bind->params[0]));
}

int bind_eos_set_event_mode(struct BindReq *bind, struct PtpRuntime *r) {
	return sprintf(bind->buffer, "{\"error\": %d}", ptp_eos_set_event_mode(r, bind->params[0]));
}

int bind_hello_world(struct BindReq *bind, struct PtpRuntime *r) {
	int len = sprintf(bind->buffer, "{\"name\": \"%s\", \"string\": \"%s\", \"params\": [", bind->name, bind->string);
	for (int i = 0; i < bind->params_length; i++) {
		char *comma = "";
		if (i) comma = ",";
		len += sprintf(bind->buffer + len, "%s%d", comma, bind->params[i]);
	}

	len += sprintf(bind->buffer + len, "]}");
	return len;
}

int bind_get_enums(struct BindReq *bind, struct PtpRuntime *r) {
	int x = sprintf(bind->buffer, "{\"error\": %d, \"resp\": [", 0);
	char *comma = "";
	for (int i = 0; i < ptp_enums_length; i++) {
		x += sprintf(bind->buffer + x, "%s{\"type\": %d, \"vendor\": %d, \"name\": \"%s\", \"value\": %d}",
			comma, ptp_enums[i].type, ptp_enums[i].vendor, ptp_enums[i].name, ptp_enums[i].value);
		comma = ",";
	}

	x += sprintf(bind->buffer + x, "]}");
	return x;
}

int bind_get_status(struct BindReq *bind, struct PtpRuntime *r) {
	return sprintf(bind->buffer, "{\"error\": 0, \"connected\": %d}", bind_connected);
}

int bind_bulb_start(struct BindReq *bind, struct PtpRuntime *r) {
	int x = 0;
	if (ptp_device_type(r) == PTP_DEV_EOS) {
		x = ptp_eos_remote_release_on(r, 1);
		if (ptp_get_return_code(r) != PTP_RC_OK) return sprintf(bind->buffer, "{\"error\": %d}", PTP_CHECK_CODE);
		x = ptp_eos_remote_release_on(r, 2);
		if (ptp_get_return_code(r) != PTP_RC_OK) return sprintf(bind->buffer, "{\"error\": %d}", PTP_CHECK_CODE);
	} else {
		x = PTP_UNSUPPORTED;
	}

	return sprintf(bind->buffer, "{\"error\": %d}", x);
}

int bind_bulb_stop(struct BindReq *bind, struct PtpRuntime *r) {
	int x = 0;
	if (ptp_device_type(r) == PTP_DEV_EOS) {
		x = ptp_eos_remote_release_off(r, 2);
		if (ptp_get_return_code(r) != PTP_RC_OK) return sprintf(bind->buffer, "{\"error\": %d}", PTP_CHECK_CODE);
		x = ptp_eos_remote_release_off(r, 1);
		if (ptp_get_return_code(r) != PTP_RC_OK) return sprintf(bind->buffer, "{\"error\": %d}", PTP_CHECK_CODE);
	} else {
		x = PTP_UNSUPPORTED;
	}

	return sprintf(bind->buffer, "{\"error\": %d}", x);
}

int bind_pre_take_picture(struct BindReq *bind, struct PtpRuntime *r) {
	return sprintf(bind->buffer, "{\"error\": %d}", ptp_pre_take_picture(r));
}

int bind_take_picture(struct BindReq *bind, struct PtpRuntime *r) {
	return sprintf(bind->buffer, "{\"error\": %d}", ptp_take_picture(r));
}

int bind_cancel_af(struct BindReq *bind, struct PtpRuntime *r) {
	int x = 0;
	if (ptp_check_opcode(r, PTP_OC_EOS_AfCancel)) {
		x = ptp_eos_cancel_af(r);
		if (ptp_get_return_code(r) != PTP_RC_OK) return sprintf(bind->buffer, "{\"error\": %d}", PTP_CHECK_CODE);
	} else {
		x = PTP_UNSUPPORTED;
	}

	return sprintf(bind->buffer, "{\"error\": %d}", x);
}

int bind_mirror_up(struct BindReq *bind, struct PtpRuntime *r) {
	int x = 0;
	if (ptp_device_type(r) == PTP_DEV_EOS) {
		x = ptp_eos_set_prop_value(r, PTP_PC_EOS_VF_Output, 3);
	} else {
		x = PTP_UNSUPPORTED;
	}

	return sprintf(bind->buffer, "{\"error\": %d}", x);
}

int bind_mirror_down(struct BindReq *bind, struct PtpRuntime *r) {
	int x = 0;
	if (ptp_device_type(r) == PTP_DEV_EOS) {
		x = ptp_eos_set_prop_value(r, PTP_PC_EOS_VF_Output, 0);
	} else {
		x = PTP_UNSUPPORTED;
	}

	return sprintf(bind->buffer, "{\"error\": %d}", x);
}

int bind_get_return_code(struct BindReq *bind, struct PtpRuntime *r) {
	int curr = sprintf(bind->buffer, "{\"error\": 0, \"code\": %d, \"params\": [", ptp_get_return_code(r));
	for (int i = 0; i < ptp_get_param_length(r); i++) {
		char *comma = "";
		if (i) comma = ",";
		curr += sprintf(bind->buffer + curr, "%s%u", comma, ptp_get_param(r, i));
	}

	return sprintf(bind->buffer, "]}");
}

int bind_reset(struct BindReq *bind, struct PtpRuntime *r) {
	return sprintf(bind->buffer, "{\"error\": %d}", ptp_device_reset(r));
}

int bind_get_thumbnail(struct BindReq *bind, struct PtpRuntime *r) {
	int x = ptp_get_thumbnail(r, bind->params[0]);

	if (x) {
		return sprintf(bind->buffer, "{\"error\": %d}", x);
	}

	if (ptp_get_payload_length(r) <= 0) {
		return sprintf(bind->buffer, "{\"error\": %d}", PTP_CHECK_CODE);
	}

	int curr = sprintf(bind->buffer, "{\"error\": 0, \"jpeg\": [");

	for (int i = 0; i < ptp_get_payload_length(r); i++) {
		char *comma = "";
		if (i) comma = ",";
		curr += sprintf(bind->buffer + curr, "%s%u", comma, ((uint8_t *)ptp_get_payload(r))[i]);		
	}

	curr += sprintf(bind->buffer + curr, "]}");
	return curr;
}

int bind_get_partial_object(struct BindReq *bind, struct PtpRuntime *r) {
	int x = ptp_get_partial_object(r, bind->params[0], bind->params[1], bind->params[2]);

	if (x) {
		return sprintf(bind->buffer, "{\"error\": %d}", x);
	}

	if (ptp_get_payload_length(r) <= 0) {
		return sprintf(bind->buffer, "{\"error\": %d}", PTP_CHECK_CODE);
	}

	int curr = sprintf(bind->buffer, "{\"error\": 0, \"data\": [");

	for (int i = 0; i < ptp_get_payload_length(r); i++) {
		char *comma = "";
		if (i) comma = ",";
		curr += sprintf(bind->buffer + curr, "%s%u", comma, ((uint8_t *)ptp_get_payload(r))[i]);		
	}

	curr += sprintf(bind->buffer + curr, "]}");
	return curr;
}

int bind_download_file(struct BindReq *bind, struct PtpRuntime *r) {
	FILE *f = fopen(bind->string, "wb");
	int x = ptp_download_object(r, bind->params[0],f, 0x100000);
	fclose(f);
	if (x < 0) {
		return sprintf(bind->buffer, "{\"error\": %d}", -1);
	} else {
		return sprintf(bind->buffer, "{\"error\": 0, \"read\": %d}", x);
	}
}

struct RouteMap {
	char *name;
	int (*call)(struct BindReq *, struct PtpRuntime *);
}routes[] = {
	{"ptp_hello_world", bind_hello_world},
	{"ptp_status", bind_status},
	{"ptp_reset", bind_reset},
	{"ptp_init", bind_init},
	{"ptp_connect", bind_connect},
	{"ptp_disconnect", bind_disconnect},
	{"ptp_open_session", bind_open_session},
	{"ptp_close_session", bind_close_session},
	{"ptp_get_device_info", bind_get_device_info},

	{"ptp_pre_take_picture", bind_pre_take_picture},
	{"ptp_take_picture", bind_take_picture},

	{"ptp_bulb_start", bind_bulb_start},
	{"ptp_bulb_stop", bind_bulb_stop},

	{"ptp_eos_set_remote_mode", bind_eos_set_remote_mode},
	{"ptp_eos_set_event_mode", bind_eos_set_event_mode},

	{"ptp_cancel_af", bind_cancel_af},

	{"ptp_mirror_up", bind_mirror_up},
	{"ptp_mirror_down", bind_mirror_down},
	{"ptp_drive_lens", bind_drive_lens},
	{"ptp_get_liveview_frame", bind_get_liveview_frame},
	{"ptp_get_liveview_type", bind_get_liveview_type},
	{"ptp_get_liveview_frame.jpg", bind_get_liveview_frame_jpg},
	{"ptp_ml_get_bmp_lv", bind_ml_get_bmp_lv},
	{"ptp_ml_init_bmp_lv", bind_ml_init_bmp_lv},
	{"ptp_init_liveview", bind_liveview_init},
	{"ptp_deinit_liveview", bind_liveview_deinit},
	{"ptp_get_device_type", bind_get_device_type},
	{"ptp_get_events", bind_get_events},
	{"ptp_get_all_props", bind_get_all_props},
	{"ptp_set_property", bind_set_property},
	{"ptp_get_enums", bind_get_enums},
	{"ptp_get_status", bind_get_status},
	{"ptp_get_return_code", bind_get_return_code},
	{"ptp_get_storage_ids", bind_get_storage_ids},
	{"ptp_get_storage_info", bind_get_storage_info},
	{"ptp_get_object_handles", bind_get_object_handles},
	{"ptp_get_object_info", bind_get_object_info},
	{"ptp_get_thumbnail", bind_get_thumbnail},
	{"ptp_get_partial_object", bind_get_partial_object},
	{"ptp_download_file", bind_download_file},
	{"ptp_custom", bind_custom},
};

static int isDigit(char c) {return c >= '0' && c <= '9';}
void bind_parse(struct BindReq *br, char *req) {
	br->params_length = 0;
	br->bytes_length = 0;

	memset(br->params, 0, sizeof(int) * BIND_MAX_PARAM);
	memset(br->name, 0, BIND_MAX_NAME);
	memset(br->bytes, 0, BIND_MAX_BYTES);
	memset(br->string, 0, BIND_MAX_STRING);

	int c = 0;
	int s = 0;
	while (req[c] != '\0') {
		if (s == 0) {
			// Parse request name
			if (c >= BIND_MAX_NAME) return;
			br->name[c] = req[c];
		} else if (s == 1) {
			if (isDigit(req[c]) || req[c] == '-') {
				// Parse base 10 integer
				if (br->params_length >= BIND_MAX_PARAM) { return; }
				int negative = 0;
				if (req[c] == '-') { negative = 1; c++; }
				while (isDigit(req[c])) {
					br->params[br->params_length] *= 10;
					br->params[br->params_length] += req[c] - '0';
					c++;
				}
				if (negative) br->params[br->params_length] *= -1;
				br->params_length++;
			} else if (req[c] == '\"') {
				// Parse string, wherever it is
				c++;
				int c2 = 0;
				while (req[c] != '\"') {
					br->string[c2] = req[c];
					c2++;
					c++;
					if (c2 >= BIND_MAX_STRING) { return; }
				}
				br->string[c2] = '\0';
				c++;
			}
		} else if (s == 2) {
			if (br->params_length >= BIND_MAX_BYTES) { return; }
			while (isDigit(req[c])) {
				br->bytes[br->params_length] *= 10;
				br->bytes[br->bytes_length] += req[c] - '0';
				c++;
			}
			br->bytes_length++;
		}

		if (req[c] == '\0') return;

		if (req[c] == ';') {
			if (s == 0) {
				br->name[c] = '\0';
			}

			s++;
		}

		c++;
	}
}

// See DOCS.md for documentation
int bind_run(struct PtpRuntime *r, char *req, char *buffer, int max) {
	if (buffer == NULL) {
		return -1;
	}

	struct BindReq bind;
	memset(&bind, 0, sizeof(struct BindReq));
	bind.buffer = buffer;
	bind.max = max;

	bind_parse(&bind, req);

	for (int i = 0; i < (int)(sizeof(routes) / sizeof(struct RouteMap)); i++) {
		if (!strcmp(routes[i].name, bind.name)) {
			return routes[i].call(&bind, r);
		}
	}

	return -1;
}

int bind_run_req(struct PtpRuntime *r, struct BindReq *bind, char *buffer, int max) {
	if (buffer == NULL) {
		return -1;
	}

	for (int i = 0; i < (int)(sizeof(routes) / sizeof(struct RouteMap)); i++) {
		if (!strcmp(routes[i].name, bind->name)) {
			return routes[i].call(bind, r);
		}
	}

	return -1;	
}
