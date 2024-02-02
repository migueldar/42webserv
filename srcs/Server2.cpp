#include "webserv.hpp"

Server::Server(std::vector<std::string>& s, std::vector<Location>& l) {
	for (size_t i = 0; i < s.size(); i++) {
		routes[s[i]] = l[i];
	}
}

const Location& Server::getLocation(std::string locationName) const {
	return (routes.at(locationName));
}
