#include "Request.hpp"
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

	parseFirstLine(first_line);

}

Request::~Request() {}

bool Request::addToBody(std::string data) {
	body += data;

	//must check wether the read of the req is done, use content-length header
	return true;
}

void Request::parseFirstLine(std::string& line) {
	std::string first, second, third;
	std::string::const_iterator it = line.begin();

	while (it != line.end() && *it != ' ') {
		first += *it;
		it++;
	}
	if (it == line.end())
		throw BadRequest();
	if (first == "GET")
		method = GET;
	else if (first == "POST")
		method = POST;
	else if (first == "DELETE")
		method = DELETE;
	else
		throw MethodNotAllowed();

	while (it != line.end() && *it != ' ') {
		second += *it;
		it++;
	}
	if (it == line.end())
		throw BadRequest();
	//need to check location correctness before this, also maybe divide between location and query

	location = second;

	while (it != line.end() && *it != ' ') {
		third += *it;
		it++;
	}
	if (it == line.end())
		throw BadRequest();

	//need to parse req
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
	o << "Location: " << std::endl << "    " << prt.location << std::endl << std::endl;
	o << "Version: " << std::endl << "    " << prt.version << std::endl << std::endl;
	o << "Headers: " << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = prt.headers.begin(); it != prt.headers.end(); it++)
		o << "    " << it->first << ": " << it->second << std::endl; 

	o << std::endl;
	o << "Body: " << std::endl << "    " << prt.body << std::endl << std::endl;
	return (o);
}
