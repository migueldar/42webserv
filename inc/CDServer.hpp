#ifndef CDSERVER_HPP
#define CDSERVER_HPP

#include <map>
#include <vector>
#include <iostream>
#include "CDLocation.hpp"

// src/paser/CDServer.cpp
class Server {
    public:
        //ATRIBUTES------------------------------------------------------------------·#
        //PARSE MAX BODYSIZE
        unsigned long maxBodySize;
        std::string serverName;        

        //METHODS--------------------------------------------------------------------·#
        Server();
        Server(std::vector<std::string>& s, std::vector<Location>& l);
        Server(const Server &other);
        Server& operator=(const Server& other);

        void addErrorPage(const std::string& statusCode, const std::string& htmlRoute);

        void addLocation(const std::string& path, const Location& location);

        long getNumRoutes();

        const std::map<std::string, Location>& getLocations(void) const;
        
        std::string getPageStatus(const std::string& statusCode) const;

        std::vector<std::string> getKeysRoutes() const;

        const std::map<std::string, Location>& getRoutes() const;

        const std::map<std::string, std::string>& getErrPages() const;

        bool existsLocationByRoute(const std::string& path) const;

        const Location& getLocation(std::string locationName) const;

    
    private:
        //ATRIBUTES------------------------------------------------------------------·#
        //Key:Status | Value:Route_to_html
        std::map<std::string, std::string> errPages;
        // Key:LocationPath | Value:classLocation 
        std::map<std::string, Location> routes;
};

#endif