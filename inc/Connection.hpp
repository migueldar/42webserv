#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "parser.hpp"

class Connection {
	public:
		int sock;
		std::vector<Server> &servers;
		//set to true when something has been recieved but not the full request
		bool parsing;
		//we probably need timeout variable here which will be set when the request is written the first time and must be taken into count, however, how can we take it into count if we are suck in the poll? (maybe another process has to take care of it?)
		

		Connection(int socket, std::vector<Server> &servers);
		Connection(const Connection &other);
		~Connection();
		bool operator==(const Connection &other) const;
		int handleEvent(struct pollfd &pollfd) const;

	private:
};

#endif
