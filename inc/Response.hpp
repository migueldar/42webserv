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
			ERROR_RESPONSE_PAGE,
            GET_RESPONSE,
        };

        enum statusCode{
            _200,
			_201,
			_308,
			_400,
			_403,
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

        Response(std::string port, const Server& server, Request& req);
        // Response(const Response& other);
        ~Response();

        SecondaryFd prepareResponse(int err);
        std::string getPartHttpResponse();
		bool done();
        const Location& getLocationByRoute(const Server& server);

        bool checkCgiTokens(const std::string &localFilePath);
		statusCode checkAccess(std::string path, enum methodsEnum method, bool autoIndex, bool& isDir);

        void handleStartPrepingRes();
		void handleProcessingRes();
        void handleWaitingForCgi(int err);
        void handleGetResponse();
        void handleGetAutoIndex();
    
        void handleBadResponse();
		void handleBadResponsePage();

        // Response& operator=(const Response& other);
    private:
        Request&       		req;
        stringWrap			body;
        stringWrap			httpResponse;
        std::string			header;
		std::string			reconstructPath;
        std::string			locationPath;
        std::string			localFilePath;
        std::string			cgiToken;
        std::string			port;
        std::string         uploadFilePath;
		stringWrap			fileContent;

		statusCode			statusCodeVar;						
        responseStages   	status;

        const Location&		loc;
		const Server&		server;
        CgiHandler*			newCgi;

		SecondaryFd			secFd;
};

#endif