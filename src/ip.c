#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "ptp.h"

int ip_connect(char *address, int port) {
	int sockfd;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	// create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) {
		perror("ERROR opening socket");
		return -1;
	}

	// get server address
	server = gethostbyname(address);

	if (server == NULL) {
		fprintf(stderr, "ERROR, no such host\n");
		return -1;
	}

	// set up server address
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
	serv_addr.sin_port = htons(port);

	// connect to server
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
		perror("ERROR connecting");
		return -1;
	}

	return sockfd;
}

int ip_recieve() {
	
}

int ip_init_requests(int socket) {
	struct PtpInitReq req;
	memset(&req, 0, sizeof(struct PtpInitReq));
	req.header.length = sizeof(struct PtpInitReq);
	req.header.type = PTPIP_INIT_COMMAND_REQ;
	strcpy(req.guid, "cl_temp_guid");

	int rc = send(socket, &req, req.header.length, 0);
	if (rc == (int)req.header.length) {
		return 1;
	}

	return 0;
}

int ptpip_device_init(struct PtpRuntime *r) {
	for (int i = 0; i < 30; i++) {
		char buf[64];
		snprintf(buf, 64, "192.168.2.%d" , i);
		int fd = ip_connect(buf, 15740);
		if (fd == -1) {
			printf("Fail %s\n", buf);
		} else {
			printf("Connected to %s\n", buf);
			return 0;
		}
	}

	return 0;
}
