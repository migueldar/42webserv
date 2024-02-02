#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "parser2.hpp"

class Connection {
	public:
		int		socket;
		Server&	server;
		short	events;

		Connection(int socket, Server& serv);
		~Connection();

	private:

};


#endif
