#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <camlib.h>
#include <ptp.h>

int set_nonblocking_io(int sockfd, int enable) {
	int flags = fcntl(sockfd, F_GETFL, 0);
	if (flags == -1)
		return -1;

	if (enable)
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;

	return fcntl(sockfd, F_SETFL, flags);
}

int ptpip_connect(struct PtpRuntime *r, char *addr, int port) {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) {
		PTPLOG("Failed to create socket\n");
		return -1;
	}

	if (set_nonblocking_io(sockfd, 1) < 0) {
		close(sockfd);
		PTPLOG("Failed to set non-blocking IO\n");
		return -1;
	}

	struct sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	if (inet_pton(AF_INET, addr, &(sa.sin_addr)) <= 0) {
		close(sockfd);
		PTPLOG("Failed to convert IP address\n");
		return -1;
	}

	if (connect(sockfd, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
		if (errno != EINPROGRESS) {
			close(sockfd);
			PTPLOG("Failed to connect\n");
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
		int so_error = 0;
		socklen_t len = sizeof(so_error);
		if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len) < 0) {
			close(sockfd);
			PTPLOG("Failed to get socket options\n");
			return -1;
		}

		if (so_error == 0) {
			PTPLOG("Connection established %s:%d (%d)\n", addr, port, sockfd);
			set_nonblocking_io(sockfd, 0);
			r->fd = sockfd;
			return 0;
		}
	}

	close(sockfd);
	PTPLOG("Failed to connect\n");
	return -1;
}

int ptpip_close(struct PtpRuntime *r) {
	close(r->fd);
	return 0;
}

int ptpip_send_bulk_packet(struct PtpRuntime *r, void *data, int sizeBytes) {
	int result = write(r->fd, data, sizeBytes);
	if (result < 0) {
		return -1;
	} else {
		return result;
	}
}

int ptpip_recieve_bulk_packet(struct PtpRuntime *r, void *data, int sizeBytes) {
	int result = read(r->fd, data, sizeBytes);
	if (result < 0) {
		return -1;
	} else {
		return result;
	}
}
