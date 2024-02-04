#include "PollHandler.hpp"

PollHandler::PollHandler(): fds(NULL) {}

PollHandler::~PollHandler() {}

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

void PollHandler::removeConnection(const Connection &c) {
	removeFromFds(c.sock);
	connections.erase(std::find(connections.begin(), connections.end(), c));
}

// void PollHandler::addToFds(int fd, fdType type) {
// 	struct pollfd aux;
// 	aux.fd = fd;
// 	if (type == LISTENER)
// 		aux.events = POLLIN;
// 	else
// 		aux.events = POLLIN | POLLOUT;

// 	size_t prev_size = listeners.size() + connections.size();
// 	struct pollfd* aux_ptr = new struct pollfd[prev_size + 1];
// 	int i = 0;
// 	while (i < prev_size && fds[i].events == POLLIN) {
// 		aux_ptr[i] = fds[i];
// 		i++;
// 	}
// 	aux_ptr[i] = aux;
// 	i++;
// 	while (i < prev_size + 1) {
// 		aux_ptr[i] = fds[i - 1];
// 		i++;
// 	}

// 	delete[] fds;
// 	fds = aux_ptr;
// }

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

int PollHandler::pollMode() {
	int			status = poll(fds, listeners.size() + connections.size(), -1);
	int			i = 0;

	(void) status;
	//check listeners
	std::cout << "Checking listeners" << std::endl;
	for (std::vector<Listener>::const_iterator it = listeners.begin(); it != listeners.end(); it++) {
		if (fds[i].revents) {
			Connection c(it->handleEvent(fds[i].events));
			addConnection(c);
		}
		i++;
	}

	//check connections
	std::cout << "Checking connections" << std::endl;
	int remove;
	for (std::vector<Connection>::const_iterator it = connections.begin(); it != connections.end(); it++) {
		std::cout << "caracol" << std::endl;
		if (fds[i].revents) {
			remove = it->handleEvent(fds[i].revents);
			if (remove)
				removeConnection(*it);
		}
		i++;
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


// PollHandler& PollHandler::operator=(PollHandler const& rhs) {
// 	(void) rhs;
// 	std::cout << "PollHandler assignment operator called" << std::endl;
// 	return (*this);
// }

// std::ostream& operator<<(std::ostream& o, PollHandler const& prt) {
// 	(void) prt;
// 	return (o);
// }		
