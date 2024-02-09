#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "parser.hpp"
#include <exception>
#include <string>

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
				return "bad request";
			}
		};

		class MethodNotAllowed: public std::exception {
			virtual const char* what() const throw() {
				return "method not allowed";
			}
		};



		methodsEnum							method;
		std::string							location;
		std::string							version;
		std::map<std::string, std::string>	headers;
		std::string							body;

		Request(std::string data);
		~Request();
		//returns wether the request has been fully read already
		bool addToBody(std::string str);

		void parseFirstLine(std::string& line);
		// Request(Request const& other);
		// Request &operator=(Request const& rhs);
};

// std::ostream &operator<<(std::ostream &o, Request const &prt);

#endif
