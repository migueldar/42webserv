#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "parser.hpp"
#include "Request.hpp"
#include "Response.hpp"

#define SIZE_READ 0x1000

class Connection {
	private:
		time_t		startTime;
		//only on false when server is doing response processing
		bool		checkTime;

	public:
		int							port;
		int							sock;
		const std::vector<Server>	&servers;
		//if set to NULL, no request is being parsed currently
		Request						*req;
		//saves data which hasnt being processed by a request yet
		std::string					data;

		Connection(int port, int socket, const std::vector<Server> &servers);
		Connection(const Connection &other);
		~Connection();
		void startTimer();
		bool checkTimer() const;
		bool operator==(const Connection &other) const;
		int handleEvent(struct pollfd &pollfd);
};

#endif
