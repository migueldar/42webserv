#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "parser.hpp"
#include "Request.hpp"
#include "CDCgiHandler.hpp"
#include "SecondaryFd.hpp"

class Response {
    public:

        enum responseStages{
            START_PREPING_RES = 0,
            WAITING_FOR_CGI,
            PROCESSING_RES,
            GET_AUTO_INDEX,
            ERROR_RESPONSE,
            GET_RESPONSE,
        };

        enum statusCode{
            _200,
            _404,
			_405,
			_413,
			_500,
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

        SecondaryFd prepareResponse(int err);
        std::string getHttpResponse();
        const Location& getLocationByRoute(std::string reconstructedPath, const Server& server);

        bool checkCgiTokens(const std::string &localFilePath);
        Response::statusCode filterResponseCode(const std::string& path, methodsEnum method, bool autoIndex);

        void handleStartPrepingRes();
		void handleProcessingRes();
        long handleWaitingForCgi();
        void handleGetResponse();
        long handleGetAutoIndex();
    
        void handleBadResponse();

        // Response& operator=(const Response& other);
    private:
        std::string			header;
        std::string			body;
        std::string			httpResponse;
        std::string			reconstructPath;
        std::string			locationPath;
        std::string			localFilePath;
        std::string			cgiToken;
        std::string			port;

        unsigned long  		maxBodySizeReq;

        const Location 		&loc;
        CgiHandler     		*newCgi;
        Request        		req;

        responseStages   	status;
		statusCode			statusCodeVar;						

		SecondaryFd			secFd;
};

#endif