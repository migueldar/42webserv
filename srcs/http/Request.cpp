#include "http.hpp"
#include "parser.hpp"
#include <cstddef>
#include <algorithm>

//we will be able to handle requests which come multiple messages, but rn we assume all the request comes in one
//the way to handle is to parse  1:reqline  2:fields  3:body, checking when we hit the termination conditions
//which are  1:\r\n  2:\r\n\r\n and  3:dependant on headers, either chunked or body-len
//a way to timeout must be added as well, if a timeout is hit, 400 Bad Req
Request::Request(std::string data) {
	std::string aux;
	std::string::const_iterator it = data.begin();
	std::string::const_iterator end = data.end();

	aux = getHTTPLine(it, end);
	parseRequestLine(aux);
	it += 2;

	//fill fields
	aux = getHTTPLine(it, end);
	while (aux != "") {
		parseField(aux);
		aux = getHTTPLine(it, end);
	}

	//check correctness of fields

	//check wether body is needed

	//read body

	parseBody(data);
	
	std::cout << *this << std::endl;
}

Request::~Request() {}

bool Request::addToBody(std::string data) {
	body += data;

	//must check wether the read of the req is done, use content-length header
	return true;
}

void Request::parseMethod(std::string& str) {
	if (!isToken(str))
		throw BadRequest();
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


void Request::parseField(std::string& fieldLine) {
	std::string fieldName, fieldValue;
	std::string::const_iterator it = fieldLine.begin();

	while (it != fieldLine.end() && *it != ':') {
		fieldName += *it;
		it++;
	}
	if (*it != ':' || !isToken(fieldName))
		throw BadRequest();
	it++;

	while (it != fieldLine.end() && (*it == ' ' || *it == '\t'))
		it++;
	std::string::const_iterator rit = fieldLine.end();
	rit--;
	while (rit != it - 1 && (*rit == ' ' || *rit == '\t'))
		rit--;
	while (it < rit) {
		fieldValue += *it;
		it++;
	}
	if (!isFieldLine(fieldValue))
		throw BadRequest();

	//add to map, si ya existe poner lo que haya y sumarle , y el nuevo
}

void Request::parseBody(std::string& messageBody) {
	(void) messageBody;
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
