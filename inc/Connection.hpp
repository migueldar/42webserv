#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "parser.hpp"
#include "Request.hpp"

#define SIZE_READ 0x10000

class Connection {
	private:
		time_t		startTime;
		//only on false when server is doing response processing
		bool		checkTime;

	public:
		int					sock;
		std::vector<Server>	&servers;
		//if set to NULL, no request is being parsed currently
		Request				*req;

		Connection(int socket, std::vector<Server> &servers);
		Connection(const Connection &other);
		~Connection();
		void startTimer();
		bool checkTimer() const;
		bool operator==(const Connection &other) const;
		int handleEvent(struct pollfd &pollfd);
};

#endif
