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

		methodsEnum							method;
		std::string							target;
		std::string							address;
		std::map<std::string, std::string>	headers;
		std::map<std::string, std::string>	queryParams;
		std::string							body;
		ParseState							parsed;
		HostType							hostType;
		BodyLengthMeasure					measure;
		unsigned long						contentLength;
		std::string							errorStatus;


		Request();
		~Request();
};

std::ostream &operator<<(std::ostream &o, Request const &prt);

#endif
