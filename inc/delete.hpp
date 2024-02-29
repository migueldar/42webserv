#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "parser.hpp"
#include <exception>
#include <string>

//check max body length from server
class Request {


	private:
		std::string	rawData;

	public:

		enum ParseState {
			NOTHING,
			REQLINE,
			HEADERS,
			ALL
		};

		enum BodyLengthMeasure {
			NO_BODY,
			CHUNKED,
			CONTENT_LENGTH
		};

		enum HostType {
			IPV4,
			REGNAME
		};

		std::string							errorStatus;
		std::map<std::string, std::string>	headers;
		std::string							body;
		std::string							cgiToken;
		ParseState							parsed;
		HostType							hostType;
		BodyLengthMeasure					measure;
		unsigned long						contentLength;

		Request();
		~Request();
};

std::ostream &operator<<(std::ostream &o, Request const &prt);

#endif
