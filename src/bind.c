// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)

// Generic text bindings to PTP functions
// - The only function that is exposed is bind_run(), which returns
// valid JSON from a generic text like request
// - This is not part of the core library, and will use malloc()

// TODO: check for initialization for IO functions, can cause segfault

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <ptp.h>
#include <camlib.h>
#include <operations.h>
#include <ptpbackend.h>
#include <ptpenum.h>
#include <ptpbind.h>

// TODO: shutter half, shutter down, shutter up
// TODO: Detect Windows, Linux, Android

int bind_connected = 0;
int bind_initialized = 0;

struct RouteMap {
	char *name;
	int (*call)(struct BindReq *, struct PtpRuntime *);
};

int bind_status(struct BindReq *bind, struct PtpRuntime *r) {
	return sprintf(bind->buffer, "{\"error\": 0, \"initialized\": %d, \"connected\": %d, \"platform\": \"%s\"}",
		bind_initialized, bind_connected, CAMLIB_PLATFORM);
}

int bind_init(struct BindReq *bind, struct PtpRuntime *r) {
	if (bind_initialized) {
		free(r->data);
		if (r->di != NULL) free(r->di);
	}

	memset(r, 0, sizeof(struct PtpRuntime));
	r->data = malloc(CAMLIB_DEFAULT_SIZE);
	r->data_length = CAMLIB_DEFAULT_SIZE;
	r->di = NULL;
	bind_initialized = 1;

	//if (connected) {
		//ptp_device_close(r);
	//}

	return sprintf(bind->buffer, "{\"error\": %d, \"buffer\": %d}", 0, r->data_length);
}

int bind_connect(struct BindReq *bind, struct PtpRuntime *r) {
	// Check if uninitialized
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
	struct UintArray *arr;
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
	struct UintArray *arr;	
	printf("IN root: %d\n", bind->params[1]);
	int x = ptp_get_object_handles(r, bind->params[0], 0, bind->params[1], &arr);
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

int bind_custom_cmd(struct BindReq *bind, struct PtpRuntime *r) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_CloseSession;
	cmd.param_length = bind->params_length - 1;
	for (int i = 0; i < cmd.param_length; i++) {
		cmd.params[i] = bind->params[i];
	}

	int x = ptp_generic_send(r, &cmd);
	return sprintf(bind->buffer, "{\"error\": %d, \"resp\": %X}", x, ptp_get_return_code(r));
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

int bind_set_property(struct BindReq *bind, struct PtpRuntime *r) {
	int dev = ptp_device_type(r);
	int x = 0;

	if (strlen(bind->string) == 0) {
		if (dev == PTP_DEV_EOS) {
			x = ptp_eos_set_prop_value(r, bind->params[0], bind->params[1]);
		} else {
			x = ptp_set_prop_value(r, bind->params[0], bind->params[1]);
		}
		return sprintf(bind->buffer, "{\"error\": %d}", x);
	}

	int value = bind->params[0];

	if (!strcmp(bind->string, "aperture")) {
		if (dev == PTP_DEV_EOS) {
			x = ptp_eos_set_prop_value(r, PTP_PC_EOS_Aperture, ptp_eos_get_aperture(value, 1));
		}
	} else if (!strcmp(bind->string, "iso")) {
		if (dev == PTP_DEV_EOS) {
			x = ptp_eos_set_prop_value(r, PTP_PC_EOS_ISOSpeed, ptp_eos_get_iso(value, 1));
		}
	} else if (!strcmp(bind->string, "shutter speed")) {
		if (dev == PTP_DEV_EOS) {
			x = ptp_eos_set_prop_value(r, PTP_PC_EOS_ShutterSpeed, ptp_eos_get_shutter(value, 1));
		}
	} else if (!strcmp(bind->string, "white balance")) {
		if (dev == PTP_DEV_EOS) {
			x = ptp_eos_set_prop_value(r, PTP_PC_EOS_WhiteBalance, ptp_eos_get_white_balance(value, 1));
			x = ptp_eos_set_prop_value(r, PTP_PC_EOS_EVFWBMode, ptp_eos_get_white_balance(value, 1));
		}
	} else if (!strcmp(bind->string, "image format")) {
		if (dev == PTP_DEV_EOS) {
			int *data = ptp_eos_get_imgformat_data(value);
			if (value == IMG_FORMAT_RAW_JPEG) {
				x = ptp_eos_set_prop_data(r, PTP_PC_EOS_ImageFormat, data, 4 * 9);
			} else {
				x = ptp_eos_set_prop_data(r, PTP_PC_EOS_ImageFormat, data, 4 * 5);
			}
		}
	} else {
		return sprintf(bind->buffer, "{\"error\": %d}", PTP_UNSUPPORTED);
	}

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
	int len = sprintf(bind->buffer, "{\"name\": \"%s\", \"string\": \"%s\" params: [", bind->name, bind->string);
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
	int x = 0;
	if (ptp_device_type(r) == PTP_DEV_EOS) {
		x = ptp_eos_remote_release_on(r, 1);
		if (ptp_get_return_code(r) != PTP_RC_OK) return sprintf(bind->buffer, "{\"error\": %d}", PTP_CHECK_CODE);
	}

	return sprintf(bind->buffer, "{\"error\": %d}", x);
}

int bind_take_picture(struct BindReq *bind, struct PtpRuntime *r) {
	int x = 0;
	if (ptp_check_opcode(r, PTP_OC_InitiateCapture)) {
		x = ptp_init_capture(r, 0, 0);
	} else if (ptp_device_type(r) == PTP_DEV_EOS) {
		x = ptp_eos_remote_release_on(r, 2);
		if (ptp_get_return_code(r) != PTP_RC_OK) return sprintf(bind->buffer, "{\"error\": %d}", PTP_CHECK_CODE);
		x = ptp_eos_remote_release_off(r, 2);
		if (ptp_get_return_code(r) != PTP_RC_OK) return sprintf(bind->buffer, "{\"error\": %d}", PTP_CHECK_CODE);
		x = ptp_eos_remote_release_off(r, 1);
		if (ptp_get_return_code(r) != PTP_RC_OK) return sprintf(bind->buffer, "{\"error\": %d}", PTP_CHECK_CODE);
	} else {
		x = PTP_UNSUPPORTED;
	}

	return sprintf(bind->buffer, "{\"error\": %d}", x);
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

int bind_eos_remote_release(struct BindReq *bind, struct PtpRuntime *r) {
	int x = 0;
	if (ptp_device_type(r) == PTP_DEV_EOS) {
		switch (bind->params[0]) {
		case 1:
			x = ptp_eos_remote_release_on(r, 1);
			break;
		case 2:
			x = ptp_eos_remote_release_on(r, 2);
			break;
		case 3:
			x = ptp_eos_remote_release_off(r, 2);
			break;
		case 4:
			x = ptp_eos_remote_release_off(r, 1);
			break;
		}
		
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
	int max = 100000;
	int handle = bind->params[0];

	FILE *f = fopen(bind->string, "w");
	if (f == NULL) {
		return sprintf(bind->buffer, "{\"error\": %d}", -1);
	}

	int read = 0;
	while (1) {
		int x = ptp_get_partial_object(r, handle, read, max);
		puts("PTP GOT PARTIAL OBJ");
		if (x) {
			return sprintf(bind->buffer, "{\"error\": %d}", x);
			break;
		}

		fwrite(ptp_get_payload(r), 1, ptp_get_payload_length(r), f);
		
		if (ptp_get_payload_length(r) < max) {
			return sprintf(bind->buffer, "{\"error\": 0, \"read\": %d}", read);
			break;
		}

		read += ptp_get_payload_length(r);
	}
}

struct RouteMap routes[] = {
	{"ptp_hello_world", bind_hello_world},
	{"ptp_status", bind_status},
	{"ptp_reset", bind_reset},
	{"ptp_init", bind_init},
	{"ptp_connect", bind_connect},
	{"ptp_disconnect", bind_disconnect},
	{"ptp_open_session", bind_open_session},
	{"ptp_close_session", bind_close_session},
	{"ptp_get_device_info", bind_get_device_info},

	// TODO: start movie capture

	// TODO: soon obsolete
	{"ptp_eos_remote_release", bind_eos_remote_release},

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
//	{"ptp_custom_send", NULL},
//	{"ptp_custom_cmd", NULL},
};

static int isDigit(char c) {return c >= '0' && c <= '9';}
void bind_parse(struct BindReq *br, char *req) {
	br->params_length = 0;
	memset(br->params, 0, sizeof(int) * BIND_MAX_PARAM);
	memset(br->name, 0, BIND_MAX_NAME);
	memset(br->string, 0, BIND_MAX_STRING);

	int c = 0;
	int s = 0;
	while (req[c] != '\0') {
		// Parse request name
		if (s == 0) {
			if (c >= BIND_MAX_NAME) return;
			br->name[c] = req[c];
		} else if (s == 1) {
			// Parse base 10 integer
			if (isDigit(req[c]) || req[c] == '-') {
				int negative = 0;
				if (req[c] == '-') { negative = 1; c++; }
				while (isDigit(req[c])) {
					br->params[br->params_length] *= 10;
					br->params[br->params_length] += req[c] - '0';
					c++;
				}
				if (negative) br->params[br->params_length] *= -1;
				br->params_length++;
			// Parse string
			} else if (req[c] == '\"') {
				c++;
				int c2 = 0;
				while (req[c] != '\"') {
					br->string[c2] = req[c];
					c2++;
					c++;
				}
				br->string[c2] = '\0';
				c++;
			}
		} else if (s == 3) {
			// Payload
		}

		if (req[c] == '\0') return;

		c++;

		if (req[c] == ';') {
			if (s == 0) {
				br->name[c] = '\0';
			}

			s++;
		}
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
