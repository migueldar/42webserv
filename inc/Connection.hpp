#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "parser2.hpp"

class Connection {
	public:
		int						sock;
		std::vector<Server>&	servers;

		Connection(int socket, std::vector<Server>&	servers);
		Connection(const Connection& other);
		~Connection();
		bool operator==(const Connection& other) const;
		int handleEvent(struct pollfd& pollfd) const;

	private:

};


#endif
