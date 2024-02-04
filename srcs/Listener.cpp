#include "webserv.hpp"

Listener::Listener(int port, std::vector<Server>& servers): port(port), servers(servers) {
	sock = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	sockaddr.sin_port = htons(port);
	bind(sock, (struct sockaddr*) &sockaddr, sizeof(struct sockaddr));
}

Listener::Listener(Listener const& other): port(other.port), sock(other.sock), servers(other.servers), sockaddr(other.sockaddr) {} 

Listener::~Listener() {}

void Listener::listenMode() const {
	listen(sock, 0x7fffffff);
}

const Connection Listener::handleEvent(short revents) const {
	std::cout << "Handling listener event" << std::endl;
	(void) revents;

	struct sockaddr_in* addr = new struct sockaddr_in;
	socklen_t *len = new socklen_t;

	int new_socket = accept(sock, (struct sockaddr*) addr, len);

	if (new_socket == -1)
		perror(NULL);

	return Connection(new_socket, servers);
}
