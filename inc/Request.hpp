#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "parser.hpp"
#include <exception>
#include <string>

//check max body length from server
class Request {

	// enum t_method {
	// 	GET,
	// 	HEAD,
	// 	POST,
	// 	PUT,
	// 	DELETE,
	// 	CONNECT,
	// 	OPTIONS,
	// 	TRACE
	// };

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

		class HTTPException: public std::exception {
			virtual const char* what() const throw() = 0;
		};
		
		class BadRequest: public HTTPException {
			virtual const char* what() const throw() {
				return "400 bad request";
			}
		};

		class MethodNotAllowed: public HTTPException {
			virtual const char* what() const throw() {
				return "405 method not allowed";
			}
		};

		class URITooLong: public HTTPException {
			virtual const char* what() const throw() {
				return "414 URI Too Long";
			}
		};

		class NotImplemented: public HTTPException {
			virtual const char* what() const throw() {
				return "501 Not Implemented";
			}
		};

		class HTTPVersionNotSupported: public HTTPException {
			virtual const char* what() const throw() {
				return "505 HTTP Version Not Supported";
			}
		};

		std::string							errorStatus;
		methodsEnum							method;
		std::string							target;
		std::map<std::string, std::string>	queryParams;
		std::map<std::string, std::string>	headers;
		std::string							body;
		ParseState							parsed;
		HostType							hostType;
		BodyLengthMeasure					measure;
		long								contentLength;

		Request();
		~Request();
		void addData(std::string data);
		void parseMethod(std::string& method);
		void parseRequestTarget(std::string& target);
		void parseVersion(std::string& version);
		void parseRequestLine(std::string line);
		void parseField(std::string fieldLine);
		void parseFields(std::string fields);
		void checkFields();
		void parseBody(std::string& messageBody);
		// Request(Request const& other);
		// Request &operator=(Request const& rhs);
};

std::ostream &operator<<(std::ostream &o, Request const &prt);

#endif
