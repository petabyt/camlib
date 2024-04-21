// Include this if you wish to never use TCP/IP
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)
#include <camlib.h>
#include <ptp.h>

int ptpip_new_timeout_socket(char *addr, int port) {
	return -1;	
}

int ptpip_connect(struct PtpRuntime *r, const char *addr, int port) {
	return -1;
}

int ptpip_connect_events(struct PtpRuntime *r, const char *addr, int port) {
	return -1;
}

int ptpip_close(struct PtpRuntime *r) {
	return -1;
}

int ptpip_cmd_write(struct PtpRuntime *r, void *data, int size) {
	return -1;
}

int ptpip_cmd_read(struct PtpRuntime *r, void *data, int size) {
	return -1;
}

int ptpip_event_send(struct PtpRuntime *r, void *data, int size) {
	return -1;
}

int ptpip_event_read(struct PtpRuntime *r, void *data, int size) {
	return -1;
}
