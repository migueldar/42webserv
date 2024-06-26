#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "parser.hpp"
#include "Request.hpp"

class Response {
    private:
        std::string     header;
        std::string     body;
        std::string     httpResponse;

        const Server    &server;
        Request         req;
        std::string     reconstructPath;
        std::string     cgiToken;
        std::string     port;
    public:
        //Response();
        class NotFoundException: public std::exception {
			public:
				virtual const char* what() const throw(){
				    return "404 not found";
                }
		};

        Response(std::string port, const Server& server, Request req);
        ~Response();

        void prepareResponse();
        std::string getHttpResponse();

};

#endif