#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "parser.hpp"
#include "Request.hpp"
#include "CDCgiHandler.hpp"

class Response {
    public:

        enum responseStages{
            START_PREPING_RES = 0,
            WAITING_FOR_CGI,
            PROCESSING_RES,
            GET_RESPONSE,
        };

        enum statusCode{
            _2XX = 0,
            _4XX = -1,
        };

        //Response();
        class NotFoundException: public std::exception {
			public:
				virtual const char* what() const throw(){
				    return "404 not found";
                }
		};

        Response(std::string port, const Server& server, Request req);
        // Response(const Response& other);
        ~Response();

        int prepareResponse();
        std::string getHttpResponse();
        const Location& getLocationByRoute(std::string reconstructedPath, const Server& server);

        Response::statusCode filterResponseCode(const std::string& path, methodsEnum method);

        int handleStartPrepingRes();
        int handleWaitingForCgi();
        void handleGetResponse();
    
        void handleBadResponse();

        // Response& operator=(const Response& other);
    private:
        std::string                     header;
        std::string                     body;
        std::string                     httpResponse;
        std::string                     reconstructPath;
        std::string                     locationPath;
        std::string                     cgiToken;
        std::string                     port;

        unsigned long                   maxBodySizeReq; 

        const Location                  &loc;
        CgiHandler                      *newCgi;
        Request                         req;

        enum Response::responseStages   status;

};

#endif