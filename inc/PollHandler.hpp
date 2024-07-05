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
		void 			listenMode();
		void 			addListener(Listener& l);
		void 			addConnection(Connection &c);
		void 			removeConnection(Connection &c);
		void			removeSecondary(Connection::secondaryFd &f);
		void 			addToFds(int fd, fdType type);
		void 			removeFromFds(int fd);
		int 			pollMode();
		struct pollfd*	createFdsExtra();
		//returns connection position in list
		Connection&		findConnection(const Connection::secondaryFd& secFd);
		int				findConnectionIndex(const Connection& con) const;
		// void tester();

	private:
		//this c ptr is needed for the poll function, it will have the elements from the two vectors in the same order
		struct pollfd*							fds;
		std::vector<Listener>					listeners;
		std::list<Connection>					connections;
		std::vector<Connection::secondaryFd>	secFds;
};

#endif
