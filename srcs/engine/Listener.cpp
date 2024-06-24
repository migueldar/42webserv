#include "webserv.hpp"

Listener::Listener(unsigned port, const std::vector<Server>& servers): port(port), servers(servers) {
	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock == -1)
        throw std::runtime_error("socket error: " + std::string(strerror(errno)));
	
	int aux = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &aux, 4) == -1)
		throw std::runtime_error("setsockopt error: " + std::string(strerror(errno)));

	if (fcntl(sock, F_SETFD, O_CLOEXEC) == -1)
		throw std::runtime_error("fcntl error: " + std::string(strerror(errno)));

	if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1)
		throw std::runtime_error("fcntl error: " + std::string(strerror(errno)));

	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	sockaddr.sin_port = htons(port);

	if (bind(sock, (struct sockaddr*) &sockaddr, sizeof(struct sockaddr)) == -1)
        throw std::runtime_error("bind error: " + std::string(strerror(errno)));
}

Listener::Listener(Listener const& other): port(other.port), sock(other.sock), servers(other.servers), sockaddr(other.sockaddr) {} 

Listener::~Listener() {}

void Listener::listenMode() const {
	int status = listen(sock, 0x7fffffff);
	if (status == -1)
		std::runtime_error("listen error " + std::string(strerror(errno)));
}

const Connection Listener::handleEvent(short revents) const {
	std::cout << "Handling listener event" << std::endl;
	(void) revents;

	int new_socket = accept(sock, NULL, NULL);

	if (new_socket == -1)
		throw std::runtime_error("accept error: " + std::string(strerror(errno)));

	return Connection(new_socket, servers);
}
