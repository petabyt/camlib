// Unfinished Nikon bindings

int ptp_nikon_capture(struct PtpRuntime *r) {
	struct PtpCommand cmd;
	cmd.code = PTP_OC_NIKON_Capture;
	cmd.param_length = 0;
	return ptp_generic_send(r, &cmd);
}
