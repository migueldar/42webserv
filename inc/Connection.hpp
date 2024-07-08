#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "parser.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "SecondaryFd.hpp"
#include "StringWrapper.hpp"

#define SIZE_READ 10000

class Connection {
	private:
		time_t		startTime;
		//only on false when server is doing response processing
		bool		checkTime;

	public:
		int							port;
		int							sock;
		SecondaryFd					secFd;
		const std::vector<Server>	&servers;
		//if set to NULL, no request is being parsed currently
		Request						*req;
		Response 					*res;
		//saves data which hasnt being processed by a request yet
		stringWrap					data;

		Connection(int port, int socket, const std::vector<Server> &servers);
		Connection(const Connection &other);
		~Connection();
		void startTimer();
		bool checkTimer() const;
		bool operator==(const Connection &other) const;
		int handleEvent(struct pollfd &pollfd);
		//if nothing wants to be returned, just return fd == -1
		SecondaryFd	handleSecondaryEvent(struct pollfd &pollfd, int revent);
};

#endif
