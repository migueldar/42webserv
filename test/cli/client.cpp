#include "webserv.hpp"
 #include <unistd.h>

int main() {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in s;
	socklen_t addrlen = sizeof(s);
	std::string buffer;

	s.sin_family = AF_INET;
	s.sin_port = htons(8080);
	inet_pton(AF_INET, "127.0.0.1", &(s.sin_addr));

	if (connect(fd, (struct sockaddr*) &s, addrlen) < 0) {
		perror("connect");
		return 1;
	}

	while (1) {
		std::cout << "> ";
		std::getline(std::cin, buffer);
		if (std::cin.eof())
			break ;
		send(fd, buffer.c_str(), buffer.size(), 0);
	}

	close(fd);
}
