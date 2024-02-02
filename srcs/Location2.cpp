#include "webserv.hpp"

Location::Location() {}

Location::Location(std::string root): root(root) {}

Location::~Location() {}

Location::Location(Location const& other) {
	*this = other;
}

Location& Location::operator=(Location const& rhs) {
	root = rhs.root;
	return (*this);
}