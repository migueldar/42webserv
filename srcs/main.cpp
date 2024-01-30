#include "webserv.hpp"
#include <sys/poll.h>
#include <unistd.h>

int main(int argc, char **argv) {
	(void) argv;
	if (argc > 2)
		std::cerr << "Usage: ./webserv [configuration file]" << std::endl;

	int fd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in s;

	s.sin_family = AF_INET;
	s.sin_addr.s_addr = INADDR_ANY;
	s.sin_port = htons(8080);

	socklen_t addrlen = sizeof(s);

	bind(fd, (struct sockaddr*) &s, sizeof(struct sockaddr));


	listen(fd, 10);

	int client1_fd, client2_fd;
	//char *buffer = (char *) calloc(0x1000, 1);
	
	client1_fd = accept(fd, (struct sockaddr*) &s, &addrlen);
	client2_fd = accept(fd, (struct sockaddr*) &s, &addrlen);

	struct pollfd fds[2];

	fds[0].fd = client1_fd;
	fds[1].fd = client2_fd;

	int status;
	while (1) {
		status = poll(fds, 2, -1);
		std::cout << status << std::endl;
	}

	perror(NULL);
	return 0;
}
