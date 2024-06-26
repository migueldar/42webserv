
#include "Response.hpp"
#include "http.hpp"
#include "CDCgiHandler.hpp"

std::string Response::getHttpResponse(){
    return httpResponse;
}

Response::Response(std::string port, const Server& server, Request req): header(""), body(""), httpResponse(""), server(server), req(req), cgiToken(""), port(port) {
    //TODO REMOVE HARDCODE
    httpResponse = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\nConnection: keep-alive\r\nContent-Type: text/plain; charset=utf-8\r\n\r\n";
    reconstructPath = reconstructPathFromVec(req.target);
}

Response::~Response(){
}

void Response::prepareResponse(){
    
    try{
        const Location &loc = getLocationByRoute(reconstructPath, server);

        for (std::map<std::string, std::string>::const_iterator it = loc.cgi.begin(); it != loc.cgi.end(); ++it)
            if ((*(req.target.end())).find(it->first) != std::string::npos)
                cgiToken = it->first;
        
        if(cgiToken != ""){
            CgiHandler newCgi = CgiHandler(loc, cgiToken, port, req, req.target, req.queryParams);
            while(newCgi.handleCgiEvent() != -1);
            body = newCgi.getCgiResponse();
            std::cout << body << std::endl;
        }
    }
    catch(const Response::NotFoundException &e){
        httpResponse = "HTTP/1.1 404 OK\r\nContent-Length: 0\r\nConnection: keep-alive\r\nContent-Type: text/plain; charset=utf-8\r\n\r\n";
    }
}