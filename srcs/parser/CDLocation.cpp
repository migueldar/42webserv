#include "webserv.hpp"

Location::Location(): autoindex(false) {
    methods[GET] = 1;
    methods[POST] = 1;
    methods[DELETE] = 1;
}

Location::Location(std::string root): root(root) {}

Location::~Location() {}

Location::Location(Location const& other) {
	*this = other;
}

Location& Location::operator=(Location const& rhs) {
	root = rhs.root;
	return (*this);
}