#include "webserv.hpp"
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

	int client_fd;
	char *buffer = (char *) calloc(0x1000, 1);
	while (1) {
		std::cout << "hello" << std::endl;
		client_fd = accept(fd, (struct sockaddr*) &s, &addrlen);

		while (read(client_fd, buffer, 1) > 0) {
			write(client_fd, "t", 1);
			std::cout << buffer;
		}
	}

	perror(NULL);
	return 0;
}
