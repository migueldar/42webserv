#include "webserv.hpp"
#include <sys/poll.h>

Connection::Connection(int port, int sock, const std::vector<Server>& servers): port(port), sock(sock), servers(servers), req(NULL), res(NULL) {
	secFd.fd = -1;
	if (fcntl(sock, F_SETFD, O_CLOEXEC) == -1)
		throw std::runtime_error("fcntl error: " + std::string(strerror(errno)));
	if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1)
		throw std::runtime_error("fcntl error: " + std::string(strerror(errno)));
	startTimer();
}

Connection::Connection(Connection const& other): startTime(other.startTime), checkTime(other.checkTime), port(other.port), sock(other.sock), servers(other.servers), req(other.req), res(other.res), data(other.data) {
	secFd = other.secFd;
}

Connection::~Connection() {}

void Connection::startTimer() {
	startTime = time(NULL);
	checkTime = true;
}

// returns true if timeout
// 60s timeout
bool Connection::checkTimer() const {
	return (checkTime && startTime + 60 <= time(NULL));
}

//a connection is uniquely identified by sock fd
bool Connection::operator==(const Connection& other) const {
	return (other.sock == sock);
}

//0 means ok, 1 means remove
int Connection::handleEvent(struct pollfd& pollfd) {
	if (pollfd.revents & POLLERR) {
		std::cout << "pollerr" << std::endl;
		return 1;
	}
	else if (pollfd.revents & POLLHUP) {
		std::cout << "pollhup" << std::endl;
		return 1;
	}
	else if (pollfd.revents & POLLIN) {
		std::cout << "pollin" << std::endl;

		int read_res;
		char read_buff[SIZE_READ + 1];

		memset(read_buff, 0, SIZE_READ + 1);
		read_res = recv(sock, read_buff, SIZE_READ, 0);

		if (read_res <= 0)
			return 1;

		data += read_buff;
	}

	else if (pollfd.revents & POLLOUT) {
		std::cout << "pollout" << std::endl;
		std::string httpResponse;
		if (res != NULL) {
			httpResponse = res->getHttpResponse();
			delete res;
			res = NULL;
		}
		else
			httpResponse = "HTTP/1.1 " + req->errorStatus + "\r\nContent-Length: " + toString(req->errorStatus.size() + 9) + "\r\nConnection: close\r\n\r\n<h1>" + req->errorStatus + "</h1>";
		
		delete req;
		req = NULL;
		
		std::cout << "RESPONSE: " << httpResponse << std::endl;
		//TODO si la respuesta a mandar es muy grande (~10^6 caracteres), mandarla en cachos, a traves de varios pollin
		if (send(sock, httpResponse.c_str(), httpResponse.size(), 0) <= 0){
			return 1;
		}

		pollfd.events = POLLIN;
		startTimer();
		return 0;
	}

	if (!data.empty()) {
		if (!req)
			req = new Request();

		data = req->addData(data);
		if (req->parsed == Request::ALL) {
			checkTime = false;
			pollfd.events = 0;
		}
	}
	return 0;
}

SecondaryFd	Connection::handleSecondaryEvent(struct pollfd &pollfd, int revent) {
	if (res == NULL) {
		if (req->errorStatus != "") {
			secFd.fd = -1;
			return secFd;
		}
		else
			res = new Response(toString(port), getServerByHost(servers, req->headers.at("Host")), *req);
	}
	
	secFd = res->prepareResponse((revent & POLLERR) || (revent & POLLHUP));
	std::cout << secFd.fd << std::endl;
	std::cout << secFd.rw << std::endl;
	

	if (secFd.fd == -1)
		pollfd.events = POLLOUT;
	
	return secFd;
}
