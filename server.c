#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Max clients connected to the server
#define BACKLOGS 10
#define MAX_CLIENT_MSG_LEN 100

// Signal Handler for Child Process
void sigchld_handler(int signo);
// Sends n bytes of msg to socket
int writen(int socket, char * msg, unsigned int n);

int main(int argc, char const *argv[])
{
	int sd, new_sd;
	int err;
	int yes=1;
	int client_port;
	int bytes_recv;
	struct addrinfo addr_info, *server_info, *it;
	struct sigaction sig;
	struct sockaddr_storage client_addr;
	socklen_t sockaddr_size;
	char ip_addr[INET6_ADDRSTRLEN];
	char *msg, client_msg[MAX_CLIENT_MSG_LEN];
	void *client_ip;

	// Populate the address structure
	memset(&addr_info, 0, sizeof(addr_info));
	// IPv4 IPv6 independent
	addr_info.ai_family = AF_UNSPEC;
	// Use TCP
	addr_info.ai_socktype = SOCK_STREAM;
	addr_info.ai_flags = AI_PASSIVE;

	// Check command line arguments
	if (argc < 2) {
		printf("Invalid usage. Try: \n");
		printf("$ ./echos <port_no>\n");
		return 1;
	}

	if ((err = getaddrinfo(NULL, argv[1], &addr_info, &server_info)) != 0) {
		fprintf(stderr, "getaddrinfo() failed: %s\n", gai_strerror(err));
		return 1;
	}

	// Iterate through output of getaddrinfo() and find a port to bind to
	for (it = server_info; it != NULL; it = it->ai_next) {
		sd = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
		if (sd == -1) {
			perror("socket()");
			continue;
		}
		if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt()");
			return 1;
		}
		if (bind(sd, it->ai_addr, it->ai_addrlen) == -1) {
			close(sd);
			perror("bind()");
			// If bind was unsuccesssful, try the next port
			// Note: In this case, there happens to be only one port and
			// so the list server_info only has one element
			continue;
		}
		break;
	}

	freeaddrinfo(server_info);

	// Check if server successfully binded to the given port
	if (it == NULL) {
		fprintf(stderr, "Server failed to bind to given port.\n");
		return 1;
	}

	// Listen on the port
	if (listen(sd, BACKLOGS) == -1) {
		perror("listen()");
		return 1;
	}

	// Register the signal handler for SIGCHLD event
	// so that the resulting process is not a zombie while exiting
	sig.sa_handler = sigchld_handler;
	sigemptyset(&sig.sa_mask);
	sig.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sig, NULL) == -1) {
		perror("sigaction()");
		return 1;
	}

	printf("Waiting for connections ....\n");

	while (1) {
		sockaddr_size = sizeof(client_addr);
		// Accept incoming connection
		new_sd = accept(sd, (struct sockaddr *)&client_addr, &sockaddr_size);
		if (new_sd == -1) {
			perror("accept()");
			continue;
		}

		// Print the client IP Address and Port Number
		if ((*(struct sockaddr *)&client_addr).sa_family == AF_INET) {
			client_ip = &(((struct sockaddr_in *)&client_addr)->sin_addr);
		} else {
			client_ip = &(((struct sockaddr_in6 *)&client_addr)->sin6_addr);
		}
		client_port = ntohs((*(struct sockaddr_in *)&client_addr).sin_port);
		inet_ntop(client_addr.ss_family, client_ip, ip_addr, INET6_ADDRSTRLEN);
		printf("Connection accepted from: %s, PORT:%d\n", ip_addr, client_port);

		// Create a child process to handle the client
		if (!fork()) {
			close(sd);
			msg = "Howdy! Welcome to 602 Echo service ...\n";
			if (send(new_sd, msg, strlen(msg), 0) == -1)
				perror("send()");
			// Wait for client to send data
			while ((bytes_recv = recv(new_sd, client_msg, MAX_CLIENT_MSG_LEN, 0)) > 0) {
				client_msg[bytes_recv] = '\0';
				// Write back the received data
				writen(new_sd, client_msg, bytes_recv);
			}
			// If client closes connection, print it on standard output
			if (bytes_recv == 0) {
				printf("Client closed connection: %s, PORT:%d\n", ip_addr, client_port);
				fflush(stdout);
			}
			// If recv() fails, output with error code 
			else {
				perror("recv()");
			}
			close(new_sd);
			exit(0);
		}

		close(new_sd);
	}
}

// SIGCHLD handler
void sigchld_handler(int signo)
{
	int saved_errno = errno;
	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = saved_errno;
}

// Writes n bytes of msg to socket
// Returns -1 on error and 0 if successful
int writen(int socket, char *msg, unsigned int n)
{
	if (send(socket, msg, n, 0) == -1) {
		perror("send()");
		return -1;
	}
	return 0;
}