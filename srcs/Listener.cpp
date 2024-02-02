#include "webserv.hpp"

Listener::Listener(int port, std::vector<Server>& servers): port(port), servers(servers) {
	sock = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	sockaddr.sin_port = htons(port);
	bind(sock, (struct sockaddr*) &sockaddr, sizeof(struct sockaddr));

	events = POLLIN;
}

//NO ESTA NI DE COÑA COMPLETO
Listener::Listener(Listener const& other): servers(other.servers) {} 

Listener::~Listener() {}

void Listener::listenMode() {
	listen(sock, 0x7fffffff);
}

void Listener::handleEvent(short revents) {
	std::cout << "hola from listener" << std::endl;
	(void) revents;
}