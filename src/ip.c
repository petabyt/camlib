// POSIX PTP/IP implementation
// Copyright 2023 by Daniel C (https://github.com/petabyt/camlib)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#ifdef WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
#else
	#include <sys/socket.h>
	#include <sys/select.h>
	#include <netinet/tcp.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#endif

#include <camlib.h>
#include <ptp.h>

#define DEBUG_BYTES() for (int i = 0; i < result; i++) { printf("%02X ", ((uint8_t *)data)[i]); } puts("");

struct PtpIpBackend {
	int fd;
	int evfd;
};

#ifdef WIN32
static int set_nonblocking_io(int fd, int enable) {
	// ...
	return 0;
}

static void set_receive_timeout(int fd, int sec) {
	DWORD x = sec * 1000;
	int rc = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (void *)&x, sizeof(x));
	if (rc < 0) {
		ptp_verbose_log("Failed to set rcvtimeo: %d", errno);
	}
}

static int get_sock_error(int fd) {
	int so_error = 0;
	socklen_t len = sizeof(so_error);
	if (getsockopt(fd, SOL_SOCKET, SO_ERROR, (void *)&so_error, &len) < 0) {
		close(fd);
		ptp_verbose_log("Failed to get socket options\n");
		return -1;
	}
	return so_error;
}
#else
static int get_sock_error(int fd) {
	int so_error = 0;
	socklen_t len = sizeof(so_error);
	if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &so_error, &len) < 0) {
		close(fd);
		ptp_verbose_log("Failed to get socket options\n");
		return -1;
	}
	return so_error;
}

static void set_receive_timeout(int fd, int sec) {
	struct timeval tv_rcv;
	tv_rcv.tv_sec = 5;
	tv_rcv.tv_usec = 0;
	int rc = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv_rcv, sizeof(tv_rcv));
	if (rc < 0) {
		ptp_verbose_log("Failed to set rcvtimeo: %d", errno);
	}
}

static int set_nonblocking_io(int fd, int enable) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		return -1;

	if (enable) {
		flags |= O_NONBLOCK;
	} else {
		flags &= ~O_NONBLOCK;
	}

	return fcntl(fd, F_SETFL, flags);
}
#endif

int ptpip_new_timeout_socket(const char *addr, int port) {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	int yes = 1;
	setsockopt(
		sockfd,
		IPPROTO_TCP,
		TCP_NODELAY,
		(char *)&yes,
		sizeof(int)
	);
	setsockopt(
		sockfd,
		IPPROTO_TCP,
		SO_KEEPALIVE,
		(char *)&yes,
		sizeof(int)
	);

	if (sockfd < 0) {
		ptp_verbose_log("Failed to create socket\n");
		return -1;
	}

	if (set_nonblocking_io(sockfd, 1) < 0) {
		close(sockfd);
		ptp_verbose_log("Failed to set non-blocking IO\n");
		return -1;
	}

	struct sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	if (inet_pton(AF_INET, addr, &(sa.sin_addr)) <= 0) {
		close(sockfd);
		ptp_verbose_log("Failed to convert IP address\n");
		return -1;
	}

	if (connect(sockfd, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
		if (errno != EINPROGRESS) {
			close(sockfd);
			ptp_verbose_log("Failed to connect to socket\n");
			return -1;
		}
	}

	// timeout handling
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(sockfd, &fdset);
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	if (select(sockfd + 1, NULL, &fdset, NULL, &tv) == 1) {
		int so_error = get_sock_error(sockfd);

		if (so_error == 0) {
			ptp_verbose_log("Connection established %s:%d (%d)\n", addr, port, sockfd);
			set_nonblocking_io(sockfd, 0); // ????
			return sockfd;
		}
	}

	close(sockfd);
	ptp_verbose_log("Failed to connect\n");
	return -1;
}

static struct PtpIpBackend *init_comm(struct PtpRuntime *r) {
	if (r->comm_backend == NULL) {
		r->comm_backend = calloc(1, sizeof(struct PtpIpBackend)); 
	}

	// Max packet size for TCP
	r->max_packet_size = 65535;

	return (struct PtpIpBackend *)r->comm_backend;
}

int ptpip_connect(struct PtpRuntime *r, const char *addr, int port, int extra_tmout) {
	int fd = ptpip_new_timeout_socket(addr, port);

	struct PtpIpBackend *b = init_comm(r);

	if (fd > 0) {
		b->fd = fd;
		r->io_kill_switch = 0;
		r->operation_kill_switch = 0;
		return 0;
	} else {
		b->fd = 0;
		return fd;
	}
}

int ptpip_connect_events(struct PtpRuntime *r, const char *addr, int port) {
	int fd = ptpip_new_timeout_socket(addr, port);

	struct PtpIpBackend *b = init_comm(r);

	if (fd > 0) {
		b->evfd = fd;
		return 0;
	} else {
		b->evfd = 0;
		return fd;
	}
}

int ptpip_close(struct PtpRuntime *r) {
	struct PtpIpBackend *b = init_comm(r);
	if (b->fd) close(b->fd);
	if (b->evfd) close(b->evfd);
	return 0;
}

int ptpip_cmd_write(struct PtpRuntime *r, void *data, int size) {
	if (r->io_kill_switch) return -1;
	struct PtpIpBackend *b = init_comm(r); // calling this seems slow?
	int result = write(b->fd, data, size);
	if (result < 0) {
		return -1;
	} else {
		return result;
	}
}

int ptpip_cmd_read(struct PtpRuntime *r, void *data, int size) {
	if (r->io_kill_switch) return -1;
	struct PtpIpBackend *b = init_comm(r);
	int result = read(b->fd, data, size);
	if (result < 0) {
		return -1;
	} else {
		return result;
	}
}

int ptpip_event_send(struct PtpRuntime *r, void *data, int size) {
	if (r->io_kill_switch) return -1;
	struct PtpIpBackend *b = init_comm(r);
	int result = write(b->evfd, data, size);
	if (result < 0) {
		return -1;
	} else {
		return result;
	}
}

int ptpip_event_read(struct PtpRuntime *r, void *data, int size) {
	if (r->io_kill_switch) return -1;
	struct PtpIpBackend *b = init_comm(r);
	int result = read(b->evfd, data, size);
	if (result < 0) {
		return -1;
	} else {
		return result;
	}
}
