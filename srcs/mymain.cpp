// #include "webserv.hpp"
// #include <sys/poll.h>
// #include <sys/socket.h>
// #include <unistd.h>
// #include <vector>
// #include <stdlib.h>
// #include <cstring>

// struct pollfd *vector_to_ptr(std::vector<pollfd>& v, int fd_listen) {
// 	struct pollfd *ret = (struct pollfd *) calloc(v.size() + 2, sizeof(struct pollfd));

// 	ret[0].fd = fd_listen;
// 	ret[0].events = POLLIN;

// 	for (int i = 0; i < (int) v.size(); i++) {
// 		ret[i + 1] = v[i];
// 		std::cout << &(v[i]) << std::endl;
// 	}
// 	return ret;
// }

// int main() {
// 	int fd_listen = socket(AF_INET, SOCK_STREAM, 0);
// 	struct sockaddr_in s;

// 	s.sin_family = AF_INET;
// 	s.sin_addr.s_addr = INADDR_ANY;
// 	s.sin_port = htons(8080);

// 	bind(fd_listen, (struct sockaddr*) &s, sizeof(struct sockaddr));
// 	listen(fd_listen, 10000);


// 	char *read_buff = (char *) calloc(0x1000, 1);
// 	struct pollfd *poll_fd;
// 	std::vector<struct pollfd> vector_poll_fd;
// 	socklen_t addrlen = sizeof(s);
// 	struct pollfd aux;

// 	while (1) {
// 		std::cout << &(vector_poll_fd[0]) << std::endl;
// 		poll_fd = vector_to_ptr(vector_poll_fd, fd_listen);
// 		poll(poll_fd, vector_poll_fd.size() + 1, -1);

// 		if (poll_fd[0].revents & POLLIN) {
// 			aux.fd = accept(poll_fd[0].fd, (struct sockaddr*) &s, &addrlen);
// 			aux.events = POLLIN;
// 			vector_poll_fd.push_back(aux);
// 			send(aux.fd, "HTTP/1.1 200 OK\r\n\
// Content-Length: 0\r\n\
// Connection: keep-alive\r\n\
// \r\n", 63, 0);
// 		}
// 		for (int i = 1; i < (int) vector_poll_fd.size() + 1; i++) {
// 			if (poll_fd[i].revents & POLLIN) {
// 				std::cout << "Meow: " << vector_poll_fd[i - 1].revents << std::endl;
// 				recv(poll_fd[i].fd, read_buff, 0x1000, 0);
// 				std::cout << i <<  ": " << read_buff << std::endl << std::endl;
// 				memset(read_buff, 0, 0x1000);
// 			}
// 			if (poll_fd[i].revents & POLLHUP) {
// 				std::cout << i << " CIAO" << std::endl;
// 				close(vector_poll_fd[i - 1].fd);
// 				vector_poll_fd.erase(vector_poll_fd.begin() + i - 1);
// 			}
// 		}
// 	}
// }

#include "webserv.hpp"
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>

Server buildSampleServer() {
	Location a("/Users/mde-arpe/CURSUS/webserv/html/miau");
	Location b("/Users/mde-arpe/CURSUS/webserv/html/data");

	std::vector<Location> locations;
	locations.push_back(a);
	locations.push_back(b);

	std::vector<std::string> paths;
	paths.push_back("/gato");
	paths.push_back("/datos");

	Server ret(paths, locations);
	return ret;
}

Server buildSampleServer2() {
	Location c("/Users/mde-arpe/CURSUS/webserv/html");

	std::vector<Location> locations;
	locations.push_back(c);

	std::vector<std::string> paths;
	paths.push_back("/");

	Server ret(paths, locations);
	return ret;
}

std::vector<Server> buildSampleServers() {
	Server ser = buildSampleServer();
	Server ser2 = buildSampleServer2();
	std::vector<Server> servs;
	servs.push_back(ser);
	servs.push_back(ser2);

	return servs;
}

int main() {
	std::vector<Server> servs = buildSampleServers();

	Listener l1(8080, servs);
	// , l2(8081, servs), l3(8082, servs);
	// Connection c1(100, servs), c2(101, servs), c3(102, servs);
	PollHandler po;

	po.addListener(l1);
	po.listenMode();

	while (1) {
		po.pollMode();
		// usleep(500000);
	}

	// po.addConnection(c3);
	// po.addConnection(c1);
	// po.addConnection(c2);


	// li.listenMode();
	// po.pollMode();

	//std::cout << "Done" << std::endl;
}
