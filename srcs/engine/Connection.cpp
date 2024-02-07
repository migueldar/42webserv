#include "webserv.hpp"
#include <sys/poll.h>

Connection::Connection(int sock, std::vector<Server>& servers): sock(sock), servers(servers) {}

Connection::Connection(Connection const& other): sock(other.sock), servers(other.servers) {}

Connection::~Connection() {}

//a connection is uniquely identified by sock fd, no need to check the server
bool Connection::operator==(const Connection& other) const {
	return (other.sock == sock);
}

//0 means ok, 1 means remove
int Connection::handleEvent(struct pollfd& pollfd) const {
	if (pollfd.revents & POLLHUP) {
		std::cout << "pollhup" << std::endl;
		return 1;
	}
	else if (pollfd.revents & POLLERR) {
		std::cout << "pollerr" << std::endl;
		return 1;
	}
	else if (pollfd.revents & POLLIN) {
		//parse req

		std::cout << "pollin" << std::endl;
		int read_res;
		char *read_buff = new char[0x1000];
		memset(read_buff, 0, 0x1000);
		//think about how to join all parts of a broken down request
		//maybe MSG_WAITALL could be used
		read_res = recv(sock, read_buff, 0x1000, 0);
		std::cout << read_buff << std::endl;
		delete[] read_buff;

		if (read_res <= 0) {
			return 0;
		}

		pollfd.events = POLLOUT;
	}
	else if (pollfd.revents & POLLOUT) {
		std::cout << "pollout" << std::endl;

		send(sock, "HTTP/1.1 200 OK\r\n\
Content-Length: 0\r\n\
Connection: keep-alive\r\n\
\r\n", 63, 0);
		pollfd.events = POLLIN;
	}


	return 0;
}