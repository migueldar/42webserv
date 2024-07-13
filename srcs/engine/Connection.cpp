#include "webserv.hpp"
#include <sys/poll.h>

Connection::Connection(int port, int sock, const std::vector<Server>& servers): port(port), sock(sock), servers(servers), req(NULL), res(NULL) {
	secFd.fd = -1;
	if (fcntl(sock, F_SETFD, O_CLOEXEC) == -1)
		throw std::runtime_error("fcntl error: " + std::string(strerror(errno)));
	if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1)
		throw std::runtime_error("fcntl error: " + std::string(strerror(errno)));
	startTimerConnection();
}

Connection::Connection(Connection const& other): startTimeConnection(other.startTimeConnection), startTimeResponse(other.startTimeResponse),\
	whichTimer(other.whichTimer), checkTimers(other.checkTimers), port(other.port), sock(other.sock), servers(other.servers),\
	req(other.req), res(other.res), data(other.data) { secFd = other.secFd; }

Connection::~Connection() {
	if (req)
		delete req;
	if (res)
		delete res;
}

void Connection::startTimerConnection() {
	startTimeConnection = time(NULL);
	whichTimer = true;
	checkTimers = true;
}

void Connection::startTimerResponse() {
	startTimeResponse = time(NULL);
	whichTimer = false;
	checkTimers = true;
}

void Connection::dontCheckTimers() {
	checkTimers = false;
}

// returns true if timeout
bool Connection::checkTimerConnection() const {
	return (checkTimers && whichTimer && startTimeConnection + CONNECTION_TIMEOUT <= time(NULL));
}

// returns true if timeout
bool Connection::checkTimerResponse() const {
	return (checkTimers && !whichTimer && startTimeResponse + RESPONSE_PROCESSING_TIMEOUT <= time(NULL));
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

		read_res = recv(sock, read_buff, SIZE_READ, 0);

		if (read_res <= 0) {
			return 1;
		}
	
		data.addData(read_buff, read_res);
	}

	else if (pollfd.revents & POLLOUT) {
		std::cout << "pollout" << std::endl;
		int lenToWrite;
		if (res != NULL) {
			char* httpResponse = res->getPartHttpResponse(lenToWrite);
			if (res->done()) {
				delete res;
				res = NULL;
				delete req;
				req = NULL;
				pollfd.events = POLLIN;
				startTimerConnection();
			}
			if (send(sock, httpResponse, lenToWrite, 0) <= 0) {
				delete[] httpResponse;
				return 1;
			}
			delete[] httpResponse;
		} else {
			std::string httpResponse = "HTTP/1.1 " + req->errorStatus + "\r\nContent-Length: " + toString(req->errorStatus.size() + 9) + "\r\nConnection: close\r\n\r\n<h1>" + req->errorStatus + "</h1>";
			delete req;
			req = NULL;
			pollfd.events = POLLIN;
			startTimerConnection();
					
			// std::cout << "RESPONSE: " << httpResponse << std::endl;
			if (send(sock, httpResponse.c_str(), httpResponse.length(), 0) <= 0)
				return 1;
		}
		return 0;
	}

	if (!data.empty() && (pollfd.events & POLLIN)) {
		if (!req)
			req = new Request();

		data = req->addData(data);
		if (req->parsed == Request::ALL) {
			startTimerResponse();
			pollfd.events = 0;
		}
	}
	return 0;
}

SecondaryFd	Connection::handleSecondaryEvent(struct pollfd &pollfd, int revent) {
	static bool hasHooped = false;

	if (res == NULL) {
		if (req->errorStatus != "") {
			secFd.fd = -1;
			pollfd.events = POLLOUT;
			return secFd;
		}
		else
			res = new Response(toString(port), getServerByHost(servers, req->headers.at("Host")), *req);
	}

	if (revent & POLLERR)
		secFd = res->prepareResponse(1);
	else if ((revent & POLLHUP) && !hasHooped) {
		hasHooped = true;
		secFd = res->prepareResponse(2);
	}
	else if ((revent & POLLIN) || (revent & POLLOUT))
		secFd = res->prepareResponse(0);

	if (secFd.fd == -1) {
		pollfd.events = POLLOUT;
		hasHooped = false;
	}
	
	return secFd;
}
