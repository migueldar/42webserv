#ifndef POLLHANDLER_HPP
#define POLLHANDLER_HPP

#include "Connection.hpp"
#include "Listener.hpp"

class PollHandler {

	public:
		PollHandler();
		~PollHandler();
		void addListener(Listener& lis);
		void addConnection(Connection &con);
		int pollMode();

	private:
		std::vector<Listener>	listeners;
		std::vector<Connection>	connections;
};

#endif
