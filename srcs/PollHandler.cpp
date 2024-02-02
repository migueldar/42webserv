#include "PollHandler.hpp"

PollHandler::PollHandler() {}

PollHandler::~PollHandler() {}

void PollHandler::addListener(Listener &p) {
	listeners.push_back(p);
}

void PollHandler::addConnection(Connection &c) {
	connections.push_back(c);
}

int PollHandler::pollMode() {
	
	for (std::vector<Listener>::const_iterator it = listeners.begin(); it != listeners.end(); it++) {
		
	}

	return 1;
}

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
