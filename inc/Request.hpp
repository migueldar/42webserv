#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "parser.hpp"
#include <exception>
#include <string>

//check max body length from server
//check max req line length
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

	public:

		class BadRequest: public std::exception {
			virtual const char* what() const throw() {
				return "400 bad request";
			}
		};

		class MethodNotAllowed: public std::exception {
			virtual const char* what() const throw() {
				return "405 method not allowed";
			}
		};

		class URITooLong: public std::exception {
			virtual const char* what() const throw() {
				return "414 URI Too Long";
			}
		};

		class HTTPVersionNotSupported: public std::exception {
			virtual const char* what() const throw() {
				return "505 HTTP Version Not Supported";
			}
		};


		methodsEnum							method;
		std::string							target;
		std::map<std::string, std::string>	queryParams;
		std::map<std::string, std::string>	headers;
		std::string							body;

		Request(std::string data);
		~Request();
		//returns wether the request has been fully read already
		bool addToBody(std::string str);

		void parseRequestLine(std::string& line);
		void parseMethod(std::string& method);
		void parseRequestTarget(std::string& target);
		void parseVersion(std::string& version);
		// Request(Request const& other);
		// Request &operator=(Request const& rhs);
};

std::ostream &operator<<(std::ostream &o, Request const &prt);

#endif
