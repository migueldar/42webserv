#include "http.hpp"
#include "parser.hpp"
#include <cstddef>
#include <algorithm>

Request::Request(): rawData(""), target(""), body(""), parsed(NOTHING) {}

Request::~Request() {}

void Request::addData(std::string data) {
	size_t found;

	rawData += data;
	try {
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
			if (measure == NO_BODY)
				parsed = ALL;
			else if (measure == CONTENT_LENGTH) {
				if (rawData.length() == contentLength) {
					parsed = ALL;
					body = rawData;
					rawData.clear();
				}
			}
			else {
				found = rawData.find_last_of("\r\n\r\n");
				if (found != std::string::npos)	{
					if (found != rawData.length() - 4)
						throw BadRequest();
					parseChunkedBody(rawData.substr(0, found + 2));
					rawData.clear();
				}
			}
		}
	} catch (const HTTPException& e) {
		errorStatus = e.what();
		parsed = ALL;
	}
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

	target = parsePctEncoding(str);
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
	size_t found;
	size_t prev_found = 0;

	found = fields.find("\r\n", prev_found);
	while (found != std::string::npos) {
		parseField(fields.substr(prev_found, found - prev_found));
		prev_found = found + 2;
		found = fields.find("\r\n", prev_found);
	}
}

void Request::checkFields() {
	std::cout << "start checkFields" << std::endl;
	if (headers.count("Host") != 1 || !isHostHeader(headers["Host"]))
		throw BadRequest();
	else {
		if (isIPV4(headers["Host"].substr(0, headers["Host"].find(":"))))
			hostType = IPV4;
		else
			hostType = REGNAME;
		headers["Host"] = parsePctEncoding(headers["Host"]);
	}
	if (headers.count("Transfer-Encoding") != 1) {
		if (headers.count("Content-Length") != 1) {
			measure = NO_BODY;
			return ;
		}
		if (!isAllDigits(headers["Content-Length"]) || headers["Content-Length"].length() > 18)
			throw BadRequest();
		measure = CONTENT_LENGTH;
		contentLength = std::stol(headers["Content-Length"]);
	}
	else {
		if (toLower(headers["Transfer-Encoding"]) != "chunked")
			throw NotImplemented();
		measure = CHUNKED;
	}
	std::cout << "end checkFields" << std::endl;
}

void Request::parseChunkedBody(std::string messageBody) {
	std::cout << "parseChunked start" << std::endl;

	size_t		counter = 0;
	size_t		len;
	size_t		found;
	std::string	lenHex;
	std::string chunkBody;

	while (messageBody[counter]) {
		lenHex.clear();
		while (isxdigit(messageBody[counter])) {
			lenHex += messageBody[counter];
			counter++;
		}
		if (lenHex.length() == 0)
			throw BadRequest();
		try {
			len = hexStringToLong(lenHex); 
		} catch (const std::exception& _) {
			throw BadRequest();
		}

		found = messageBody.find("\r\n", counter);
		std::cout << len << std::endl;
		if (found == std::string::npos)
			throw BadRequest();
		if (len == 0)
			break ;
		counter = found + 2;

		chunkBody = messageBody.substr(counter, len);
		if (chunkBody.length() != len || messageBody[counter + len] != '\r' || messageBody[counter + len + 1] != '\n')
			throw BadRequest();
		body += chunkBody;
		counter += len + 2;
	}

	if (!messageBody[counter])
		throw BadRequest();

	std::cout << "parseChunked end" << std::endl << std::endl;
}

std::ostream& operator<<(std::ostream& o, Request const& prt) {
	o << "Method: ";
	switch (prt.method) {
		case GET:
			o << "GET";
			break ;
		case POST:
			o << "POST";
			break ;
		case DELETE:
			o << "DELETE";
			break ;
	}
	o << std::endl << std::endl;
	o << "Target: " << prt.target << std::endl << std::endl;
	o << "Headers: " << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = prt.headers.begin(); it != prt.headers.end(); it++)
		o << "    " << it->first << ": " << it->second << std::endl; 

	o << std::endl;
	o << "Body: " << std::endl << "    " << prt.body << std::endl << std::endl;
	o << "BodyLengthMeasure: ";
	switch (prt.measure) {
		case Request::NO_BODY:
			o << "NO_BODY";
			break ;
		case Request::CHUNKED:
			o << "CHUNKED";
			break ;
		case Request::CONTENT_LENGTH:
			o << "CONTENT_LENGTH";
			break ;
	} 
	o << std::endl << std::endl;
	o << "Content-Length: " << prt.contentLength << std::endl << std::endl;
	o << "HostType: ";
	switch (prt.hostType) {
		case Request::IPV4:
			o << "IPV4";
			break ;
		case Request::REGNAME:
			o << "REGNAME";
			break ;
	}
	o << std::endl << std::endl;
	o << "ErrorStatus: " << prt.errorStatus << std::endl << std::endl;
	o << "Parsed: " << prt.parsed << std::endl << std::endl;
	return (o);
}
