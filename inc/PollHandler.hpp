#ifndef POLLHANDLER_HPP
#define POLLHANDLER_HPP

#include "Connection.hpp"
#include "Listener.hpp"
#include <poll.h>
#include <algorithm>
#include <vector>
#include <list>

class PollHandler {

	enum fdType {
		LISTENER,
		CONNECTION
	};

	public:
		PollHandler();
		~PollHandler();
		void addListener(Listener& l);
		void listenMode();
		void addConnection(Connection &c);
		void removeConnection(Connection &c);
		void addToFds(int fd, fdType type);
		void removeFromFds(int fd);
		int pollMode();
		// void tester();

	private:
		//this c ptr is needed for the poll function, it will have the elements from the two vectors in the same order
		struct pollfd*			fds;
		std::vector<Listener>	listeners;
		std::list<Connection>	connections;
};

#endif
