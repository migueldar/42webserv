#include "http.hpp"
#include "parser.hpp"
#include <cstddef>
#include <algorithm>

Request::Request(): startTime(0), target(""), body(""), parsed(NOTHING) {}

// Request::Request(std::string data) {
// 	std::string aux;
// 	std::string::const_iterator it = data.begin();
// 	std::string::const_iterator end = data.end();

// 	aux = getHTTPLine(it, end);
// 	parseRequestLine(aux);

// 	//fill fields
// 	aux = getHTTPLine(it, end);
// 	while (aux != "") {
// 		parseField(aux);
// 		aux = getHTTPLine(it, end);
// 	}

// 	//check correctness of fields

// 	//check wether body is needed

// 	//read body

// 	parseBody(data);
	
// 	std::cout << *this << std::endl;
// }

Request::~Request() {}

//we will be able to handle requests which come multiple messages, but rn we assume all the request comes in one
//the way to handle is to parse  1:reqline  2:fields  3:body, checking when we hit the termination conditions
//which are  1:\r\n  2:line with only\r\n and  3:dependant on headers, either chunked or body-len
//a way to timeout must be added as well, if a timeout is hit, 400 Bad Req
//remember to try catch for HTTPerrors, other (memory) errors may be handeled elsewhere
void Request::addData(std::string data) {
	size_t found;

	rawData += data;
	if (parsed == NOTHING) {
		found = rawData.find("\r\n");
		if (found != std::string::npos)	{
			parseRequestLine(rawData.substr(0, found));
			parsed = REQLINE;
			rawData = rawData.substr(found + 2);
		}
	}

	if (parsed == REQLINE) {
		found = rawData.find("\r\n");
		if (found == 0)
			throw BadRequest();
		else {
			found = rawData.find("\r\n\r\n");
			if (found != std::string::npos)	{
				parseFields(rawData.substr(0, found + 2));
				parsed = HEADERS;
				checkFields();
				rawData = rawData.substr(found + 4);
			}
		}
	}

	if (parsed == HEADERS) {

	}


	std::cout << "State: " << parsed << std::endl << *this << std::endl;
}

void Request::startTimer() {
	startTime = time(NULL);
}

// returns true if timeout
// 60s timeout
bool Request::checkTimer() {
	return (startTime + 60 <= time(NULL));
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

//CHECK FOR QUERY PARAMS
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

void Request::parseRequestLine(std::string line) {
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

void Request::parseField(std::string fieldLine) {
	std::string fieldName, fieldValue;
	std::string::const_iterator it = fieldLine.begin();

	while (it != fieldLine.end() && *it != ':') {
		fieldName += *it;
		it++;
	}
	if (*it != ':' || !isToken(fieldName))
		throw BadRequest();
	it++;
	std::cout << "field name parsed" << std::endl;

	while (it != fieldLine.end() && (*it == ' ' || *it == '\t'))
		it++;
	std::string::const_iterator rit = fieldLine.end();
	rit--;
	while (rit != it - 1 && (*rit == ' ' || *rit == '\t'))
		rit--;
	while (it <= rit) {
		fieldValue += *it;
		it++;
	}
	if (!isFieldLine(fieldValue))
		throw BadRequest();
	std::cout << "field value parsed" << std::endl;

	//value is in map
	if (headers.count(fieldName) == 1)
		headers[fieldName] = headers[fieldName].append(", " + fieldValue);
	else
		headers[fieldName] = fieldValue;
}

void Request::parseFields(std::string fields) {
	size_t found = 0;
	size_t prev_found = 0;

	found = fields.find("\r\n", prev_found);
	while (found != std::string::npos) {
		parseField(fields.substr(prev_found, found - prev_found));
		prev_found = found + 2;
		found = fields.find("\r\n", prev_found);
	}
}

//
void Request::checkFields() {
	if (headers.count("Host") != 1)
		throw BadRequest();
	if (headers.count("Transfer-Encoding") != 1) {
		if (headers.count("Content-Length") != 1)
			throw BadRequest();
		if (!isAllDigits(headers["Content-Length"]) || headers["Content-Length"].length() > 18)
			throw BadRequest();
		measure = CONTENT_LENGTH;
		contentLength = std::stol(headers["Content-Length"]);
	}
	else {
		if (headers["Transfer-Encoding"] != "chunked")
			throw NotImplemented();
		measure = CHUNKED;
	}
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
