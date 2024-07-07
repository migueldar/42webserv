#include "webserv.hpp"

PollHandler::PollHandler(): fds(NULL) {}

PollHandler::~PollHandler() { 
	if (fds) delete[] fds;
}

void PollHandler::addListener(Listener &l) {
	std::cout << "Listener Added" << std::endl;
	addToFds(l.sock, LISTENER);
	listeners.push_back(l);
}

void PollHandler::listenMode() {
	for (std::vector<Listener>::const_iterator it = listeners.begin(); it != listeners.end(); it++)
		it->listenMode();
}

void PollHandler::addConnection(Connection &c) {
	std::cout << "Connection Added" << std::endl;
	addToFds(c.sock, CONNECTION);
	connections.push_back(c);
}

void PollHandler::removeConnection(Connection &c) {
	std::cout << "Connection Removed" << std::endl;
	close(c.sock);
	removeFromFds(c.sock);
	connections.erase(std::find(connections.begin(), connections.end(), c));
	removeSecondary(c.secFd);
}

void PollHandler::removeSecondary(SecondaryFd &f) {
	if (f.fd > 0) {
		std::cout << "Secondary Removed" << std::endl;
		close(f.fd);
		secFds.erase(std::find(secFds.begin(), secFds.end(), f));
	}
}

void PollHandler::addToFds(int fd, fdType type) {
	struct pollfd aux;
	aux.fd = fd;
	aux.revents = 0;
	if (type == LISTENER)
		aux.events = POLLIN;
	else
		aux.events = POLLIN;

	size_t prev_size = listeners.size() + connections.size();
	struct pollfd* aux_ptr = new struct pollfd[prev_size + 1];
	for (size_t i = 0; i < prev_size; i++)
		aux_ptr[i] = fds[i];
	aux_ptr[prev_size] = aux;

	delete[] fds;
	fds = aux_ptr;
}

void PollHandler::removeFromFds(int fd) {
	size_t prev_size = listeners.size() + connections.size();
	struct pollfd* aux_ptr = new struct pollfd[prev_size - 1];
	int j = 0;
	for (size_t i = 0; i < prev_size; i++) {
		if (fd != fds[i].fd) {
			aux_ptr[j] = fds[i];
			j++;
		}
	}

	delete[] fds;
	fds = aux_ptr;
}

struct pollfd* PollHandler::createFdsExtra() {
	struct pollfd* ret = new struct pollfd[listeners.size() + connections.size() + secFds.size()];

	for (size_t i = 0; i < listeners.size() + connections.size(); i++)
		ret[i] = fds[i];
	for (size_t i = 0; i < secFds.size(); i++) {
		ret[i + listeners.size() + connections.size()].fd = secFds[i].fd;
		if (secFds[i].rw == 0) 
			ret[i + listeners.size() + connections.size()].events = POLLIN;
		else
			ret[i + listeners.size() + connections.size()].events = POLLOUT;
	}
	return ret;
}

void PollHandler::recoverFds(struct pollfd* fdsExtra) {
	for (size_t i = 0; i < listeners.size() + connections.size(); i++)
		fds[i] = fdsExtra[i];
}

Connection&	PollHandler::findConnection(const SecondaryFd& secFd) {
	for (std::list<Connection>::iterator it = connections.begin(); it != connections.end(); it++)
		if (it->secFd == secFd)
			return *it;
	//safeguard, will never happen
	return connections.front();
}

int PollHandler::findConnectionIndex(const Connection& con) const {
	int i = 0;

	for (std::list<Connection>::const_iterator it = connections.begin(); it != connections.end(); it++) {
		if (*it == con)
			return i;
		i++;
	}
	//safeguard, will never happen
	return -1;
}

int PollHandler::pollMode() {
	struct pollfd* fdsExtra = createFdsExtra();

	int	status = poll(fdsExtra, listeners.size() + connections.size() + secFds.size(), 1000);
	int	i = 0;

	if (status < 0)
		throw std::runtime_error("poll error: " + std::string(strerror(errno)));

	//check listeners
	std::vector<Connection> toAdd;
	if (status > 0) {
		std::cout << "Checking listeners" << std::endl;
		for (std::vector<Listener>::const_iterator it = listeners.begin(); it != listeners.end(); it++) {
			if (fdsExtra[i].revents) {
				try {
					Connection c(it->handleEvent(fdsExtra[i].revents));
					toAdd.push_back(c);
				} catch (std::exception& e) {
					std::cerr << e.what() << std::endl;
				}
			}
			i++;
		}
	}

	//check connections, also timeouts
	std::cout << "Checking connections and timeouts" << std::endl;
	std::vector<Connection> toRemove;
	std::vector<SecondaryFd> toAddSecondary;
	for (std::list<Connection>::iterator it = connections.begin(); it != connections.end(); it++) {
		if (fdsExtra[i].revents) {
			if (it->handleEvent(fdsExtra[i]))
				toRemove.push_back(*it);
			else if (it->req && it->req->parsed == Request::ALL) {
				SecondaryFd auxS = it->handleSecondaryEvent(fdsExtra[i], 0);
				if (auxS.fd > 0)
					toAddSecondary.push_back(auxS);
			}
		}
		if (it->checkTimer())
			toRemove.push_back(*it);
		i++;
	}


	//check secondary
	std::vector<SecondaryFd> toRemoveSecondary;
	if (status > 0) {
		std::cout << "Checking secondary fds" << std::endl;
		for (std::vector<SecondaryFd>::const_iterator it = secFds.begin(); it != secFds.end(); it++) {
			if (fdsExtra[i].revents) {
				Connection& con = findConnection(*it);
				//check wether connection is being removed due to some error
				if (std::count(toRemove.begin(), toRemove.end(), con) == 0) {
					SecondaryFd auxS = con.handleSecondaryEvent(fdsExtra[findConnectionIndex(con) + listeners.size()], fdsExtra[i].revents);
					toRemoveSecondary.push_back(*it);
					if (auxS.fd > 0)
						toAddSecondary.push_back(auxS);
				}
			}
			i++;
		}
	}

	recoverFds(fdsExtra);
	delete[] fdsExtra;

	for (std::vector<Connection>::iterator it = toRemove.begin(); it != toRemove.end(); it++)
		removeConnection(*it);

	for (std::vector<Connection>::iterator it = toAdd.begin(); it != toAdd.end(); it++)
		addConnection(*it);
	
	for (std::vector<SecondaryFd>::iterator it = toRemoveSecondary.begin(); it != toRemoveSecondary.end(); it++)
		removeSecondary(*it);

	for (std::vector<SecondaryFd>::iterator it = toAddSecondary.begin(); it != toAddSecondary.end(); it++) {
		std::cout << "Secondary added" << std::endl;
		secFds.push_back(*it);
	}

	return 1;
}

// void	PollHandler::tester() {
// 	for (size_t i = 0; i < listeners.size(); i++)
// 		std::cout << listeners[i].sock << std::endl;
// 	for (size_t i = 0; i < connections.size(); i++)
// 		std::cout << connections[i].sock << std::endl;

// 	std::cout << std::endl;

// 	for (size_t i = 0; i < connections.size() + listeners.size(); i++)
// 		std::cout << fds[i].fd << " " << fds[i].events << std::endl;
// }

// PollHandler::PollHandler(PollHandler const& other) {
// 	std::cout << "PollHandler copy constructor called" << std::endl;
// 	*this = other;
// }

