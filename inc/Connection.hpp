#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "parser.hpp"
#include "Request.hpp"

#define SIZE_READ 0x10000

class Connection {
	public:
		int					sock;
		std::vector<Server>	&servers;
		//if set to NULL, no request is being parsed currently
		Request				*req;

		Connection(int socket, std::vector<Server> &servers);
		Connection(const Connection &other);
		~Connection();
		bool operator==(const Connection &other) const;
		int handleEvent(struct pollfd &pollfd);

	private:
};

#endif
