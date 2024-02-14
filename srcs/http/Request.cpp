#include "http.hpp"
#include "parser.hpp"
#include <cstddef>

//to my understanding, all headers must be in the first req, so they all get parsed here
Request::Request(std::string data) {
	std::string first_line;
	std::string::const_iterator it = data.begin();

	//fill the first line //check it ending before 
	while (it != data.end() && it + 1 != data.end() && (*it) != '\r' && *(it + 1) != '\n') {
		first_line += *it;
		it++;
	}
	if (it == data.end() || it + 1 == data.end())
		throw BadRequest();

	it += 2;

	parseRequestLine(first_line);

	std::cout << *this << std::endl;
}

Request::~Request() {}

bool Request::addToBody(std::string data) {
	body += data;

	//must check wether the read of the req is done, use content-length header
	return true;
}

void Request::parseMethod(std::string& str) {
	if (str == "GET")
		method = GET;
	else if (str == "POST")
		method = POST;
	else if (str == "DELETE")
		method = DELETE;
	else
		throw MethodNotAllowed();
}

void Request::parseRequestTarget(std::string& str) {
	std::string::const_iterator it = str.begin();
	std::string aux;

	while (it != str.end()) {
		if (*it != '/')
			throw BadRequest();
		it++;
		aux.clear();
		while (it != str.end() && *it != '/') {
			aux += *it;
			it++;
		}
		if (!isSegment(aux))
			throw BadRequest();
	}

	for (it = str.begin(); it != str.end(); it++) {
		if (*it == '%') {
			target += hexToNum(it[1]) * 0x10 + hexToNum(it[2]);
			it++; it++;
		}
		else
			target += *it;
	}
}

void Request::parseVersion(std::string& str) {
	if (str.length() != 8 || str.substr(0, 5) != "HTTP/"\
		|| !isdigit(str[5]) || str[6] != '.' || !isdigit(str[7]))
		throw BadRequest();
	if (str[5] != '1' || str[7] == '0')
		throw HTTPVersionNotSupported();
}

void Request::parseRequestLine(std::string& line) {
	std::string aux;
	std::string::const_iterator it = line.begin();

	if (line.length() > 8000)
		throw URITooLong();

	while (it != line.end() && *it != ' ') {
		aux += *it;
		it++;
	}
	if (it == line.end())
		throw BadRequest();
	parseMethod(aux);
	std::cout << "method parsed" << std::endl;

	aux.clear();
	it++;
	while (it != line.end() && *it != ' ') {
		aux += *it;
		it++;
	}
	if (it == line.end())
		throw BadRequest();
	parseRequestTarget(aux);
	std::cout << "req target parsed" << std::endl;

	aux.clear();
	it++;
	while (it != line.end()) {
		aux += *it;
		it++;
	}
	parseVersion(aux);
	std::cout << "version parsed" << std::endl;
}

// Request::Request(Request const& other) {
// 	std::cout << "Request copy constructor called" << std::endl;
// 	*this = other;
// }

// Request& Request::operator=(Request const& rhs) {
// 	(void) rhs;
// 	std::cout << "Request copy assignment operator called" << std::endl;
// 	return (*this);
// }

std::ostream& operator<<(std::ostream& o, Request const& prt) {
	o << "Method: " << std::endl << "    " << prt.method << std::endl << std::endl;
	o << "Target: " << std::endl << "    " << prt.target << std::endl << std::endl;
	o << "Headers: " << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = prt.headers.begin(); it != prt.headers.end(); it++)
		o << "    " << it->first << ": " << it->second << std::endl; 

	o << std::endl;
	o << "Body: " << std::endl << "    " << prt.body << std::endl << std::endl;
	return (o);
}
