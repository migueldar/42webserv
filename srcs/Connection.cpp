#include "webserv.hpp"

Connection::Connection(int sock, std::vector<Server>& servers): sock(sock), servers(servers) {}

Connection::Connection(Connection const& other): sock(other.sock), servers(other.servers) {}

Connection::~Connection() {}

//a connection is uniquely identified by sock fd, no need to check the server
bool Connection::operator==(const Connection& other) {
	return (other.sock == sock);
}

//0 means ok, 1 means remove
int Connection::handleEvent(short revents) const {
	std::cout << "Handling connection event" << std::endl;
	if (revents & POLLHUP)
		return 1;

	if (revents & (POLLIN | POLLOUT)) {
		//parse req
		char *buff = new char[0x1000];

		recv(sock, buff, 0x1000, 0);
		std::cout << buff << std::endl;

		send(sock, "HTTP/1.1 200 OK\r\n\
Content-Length: 0\r\n\
Connection: keep-alive\r\n\
\r\n", 63, 0);
	}

	return 0;
}