#include "webserv.hpp"
#include <sys/poll.h>

Connection::Connection(int sock, std::vector<Server>& servers): sock(sock), servers(servers), req(NULL) {
	startTimer();
}

//if smt around here fails, check this, bc its not currently a full copy
Connection::Connection(Connection const& other): startTime(other.startTime), checkTime(other.checkTime), sock(other.sock), servers(other.servers), req(other.req) {}

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
	if (pollfd.revents & POLLHUP) {
		std::cout << "pollhup" << std::endl;
		return 1;
	}
	else if (pollfd.revents & POLLERR) {
		std::cout << "pollerr" << std::endl;
		return 1;
	}

	else if (pollfd.revents & POLLIN) {
		std::cout << "pollin" << std::endl;


		int read_res;
		char *read_buff = new char[SIZE_READ];

		memset(read_buff, 0, SIZE_READ);
		read_res = recv(sock, read_buff, SIZE_READ, 0);
		std::cout << read_buff << std::endl;

		if (read_res <= 0) {
			delete[] read_buff;
			return 1;
		}

		if (!req)
			req = new Request();

		req->addData(std::string(read_buff));
		delete[] read_buff;

		if (req->parsed == Request::ALL) {
			pollfd.events = POLLOUT;
			checkTime = false;
		}
	}

	//response handling should be done here, not in pollin
	else if (pollfd.revents & POLLOUT) {
		std::cout << "pollout" << std::endl;

		//probably have to handle send return value
		send(sock, "HTTP/1.1 200 OK\r\n\
Content-Length: 0\r\n\
Connection: keep-alive\r\n\
Content-Type: text/plain; charset=utf-8\r\n\
\r\n", 104, 0);
		//call Response constructor, we pass request

		startTimer();
		pollfd.events = POLLIN;
		delete req;
		req = NULL;
	}

	return 0;
}