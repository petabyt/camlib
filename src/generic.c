// Generic device-independent interface for cameras
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <camlib.h>

int ptp_set_generic_property(struct PtpRuntime *r, char *name, int value) {
	int dev = ptp_device_type(r);
	int rc = 0;
	if (!strcmp(name, "aperture")) {
		if (dev == PTP_DEV_EOS) {
			rc = ptp_eos_set_prop_value(r, PTP_PC_EOS_Aperture, ptp_eos_get_aperture(value, 1));
		}
	} else if (!strcmp(name, "iso")) {
		if (dev == PTP_DEV_EOS) {
			rc = ptp_eos_set_prop_value(r, PTP_PC_EOS_ISOSpeed, ptp_eos_get_iso(value, 1));
		}
	} else if (!strcmp(name, "shutter speed")) {
		if (dev == PTP_DEV_EOS) {
			rc = ptp_eos_set_prop_value(r, PTP_PC_EOS_ShutterSpeed, ptp_eos_get_shutter(value, 1));
		}
	} else if (!strcmp(name, "white balance")) {
		if (dev == PTP_DEV_EOS) {
			rc = ptp_eos_set_prop_value(r, PTP_PC_EOS_WhiteBalance, ptp_eos_get_white_balance(value, 1));
			rc = ptp_eos_set_prop_value(r, PTP_PC_EOS_EVFWBMode, ptp_eos_get_white_balance(value, 1));
		}
	} else if (!strcmp(name, "destination")) {
		return 0;
		//bind_capture_type = value;
	} else {
		return PTP_UNSUPPORTED;
	}

	return rc;
}

// Required for ptp_take_picture
int ptp_pre_take_picture(struct PtpRuntime *r) {
	if (ptp_device_type(r) == PTP_DEV_EOS) {
		// Shutter half down, wait up to 10s for focus 
		r->wait_for_response = 10;
		int rc = ptp_eos_remote_release_on(r, 1);
		if (rc) return rc;
	}

	return 0;	
}

int ptp_take_picture(struct PtpRuntime *r) {
	if (ptp_device_type(r) == PTP_DEV_EOS) {
		// Shutter fully down, wait up to 3s for flash to pop up
		r->wait_for_response = 3;
		int rc = ptp_eos_remote_release_on(r, 2);
		if (rc) return rc;

		rc = ptp_eos_remote_release_off(r, 2);
		if (rc) return rc;

		rc = ptp_eos_remote_release_off(r, 1);
		if (rc) return rc;

		return 0;
	}

	return PTP_UNSUPPORTED;
}

int ptp_events_json(struct PtpRuntime *r, char *buffer, int max) {
	return 0;
}

int ptp_get_all_known(struct PtpRuntime *r, struct PtpGenericEvent **s, int *length) {
	uint16_t *props = r->di->props_supported;
	int plength = r->di->props_supported_length;
	(*length) = plength;

	(*s) = malloc(sizeof(struct PtpGenericEvent) * plength);

	for (int i = 0; i < plength; i++) {
		struct PtpGenericEvent *cur = &((*s)[i]);
		memset(cur, 0, sizeof(struct PtpGenericEvent));

		cur->code = props[i];

		int rc = ptp_get_prop_value(r, props[i]);
		if (rc) return rc;

		int v = ptp_parse_prop_value(r);
		cur->value = v;
		if (v == -1) {
			continue;
		}

		// TODO: Get more props
		switch (props[i]) {
		case PTP_PC_BatteryLevel:
			cur->name = "battery";
			cur->value = v;
			break;
		}
	}

	return 0;
}
