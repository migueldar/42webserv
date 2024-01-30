#include "webserv.hpp"
#include <sys/poll.h>
#include <unistd.h>
#include <vector>
#include <stdlib.h>

struct pollfd *vector_to_ptr(std::vector<int> v, int fd_listen) {
	struct pollfd *ret = (struct pollfd *) calloc(v.size() + 2, sizeof(struct pollfd));

	ret[0].fd = fd_listen;
	ret[0].events = POLLIN;

	for (int i = 0; i < (int) v.size(); i++) {
		ret[i + 1].fd = v[i];
		ret[i + 1].events = POLLIN;
	}
	return ret;
}

int main() {
	int fd_listen = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in s;

	s.sin_family = AF_INET;
	s.sin_addr.s_addr = INADDR_ANY;
	s.sin_port = htons(8080);

	bind(fd_listen, (struct sockaddr*) &s, sizeof(struct sockaddr));
	listen(fd_listen, 10000);


	char *read_buff = (char *) calloc(0x1000, 1);
	struct pollfd *poll_fd;
	std::vector<int> vector_poll_fd;
	int aux_fd;
	socklen_t addrlen = sizeof(s);


	while (1) {
		poll_fd = vector_to_ptr(vector_poll_fd, fd_listen);
		poll(poll_fd, vector_poll_fd.size() + 1, -1);

		if (poll_fd[0].revents & POLLIN) {
			aux_fd = accept(poll_fd[0].fd, (struct sockaddr*) &s, &addrlen);
			vector_poll_fd.push_back(aux_fd);
		}
		for (int i = 1; i < (int) vector_poll_fd.size() + 1; i++) {
			if (poll_fd[i].revents & POLLIN) {
				recv(poll_fd[i].fd, read_buff, 0x1000, 0);
				std::cout << i <<  ": " << read_buff << std::endl << std::endl;
				memset(read_buff, 0, 0x1000);
			}
			if (poll_fd[i].revents & POLLHUP)
				std::cout << i << " CIAO" << std::endl;
		}
	}
}