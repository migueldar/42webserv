#include "webserv.hpp"
 #include <unistd.h>

int main() {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in s;
	socklen_t addrlen = sizeof(s);
	std::string send_buffer;

	s.sin_family = AF_INET;
	s.sin_port = htons(9090);
	inet_pton(AF_INET, "127.0.0.1", &(s.sin_addr));

	if (connect(fd, (struct sockaddr*) &s, addrlen) < 0) {
		perror("connect");
		return 1;
	}

	char *recieve_buffer = new char[0x10000];
	while (1) {
		memset(recieve_buffer, 0, 0x10000);
		std::cout << "> ";
		std::getline(std::cin, send_buffer);
		for (size_t i = 0; i < send_buffer.length(); i++) {
			if (send_buffer[i] == '?')
				send_buffer[i] = '\r';
			if (send_buffer[i] == '!')
				send_buffer[i] = '\n';
		}


		if (std::cin.eof())
			break ;
		send(fd, send_buffer.c_str(), send_buffer.size(), 0);
		if (recv(fd, recieve_buffer, 0x10000, 0) < 0)
		{
			perror("recv");
			exit(1);
		}
		std::cout << recieve_buffer << std::endl;
	}

	close(fd);
}
