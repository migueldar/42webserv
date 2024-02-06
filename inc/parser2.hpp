#ifndef PARSER2_HPP
#define PARSER2_HPP

#include <iostream>
#include <vector>
#include <map>
//#include <cstdint>
#include <limits>
#include "Listener.hpp"

class Listener;

#define METHODS_NUM 3

enum methodsEnum{
    GET,
    POST,
    DELETE
};

class Location {
	public:
		std::string  root;

		//NOT DEFAULT NEEDED
		std::string defaultPath;

		// NO DEFAULT NEEDED 
		//std::string redirectionUrl = "";

		//MANAGES ESPECIFIC RULES FOR METHODS
		bool methods[METHODS_NUM];

		//OFF predeterminado
		bool autoindex;
	

		Location(std::string root);
		Location(Location const& other);
		Location();
		~Location();
		Location& operator=(Location const& rhs);

};

class Server {

	public:
		//PARSE MAX BODYSIZE
		uint32_t maxBodySize;

		// FULL 4 bytes AUTOASIGNABLE
		uint32_t serverID; 

		// MUST HAVE DEFAULT    
		std::string serverName;

		Server(std::vector<std::string>& s, std::vector<Location>& l);
		const Location& getLocation(std::string locationName) const;


	private:
		//Key:Status | Value:Route_to_html
		std::map<uint16_t, std::string> parserErrPages;

		// Key:LocationPath | Value:classLocation 
		std::map<std::string, Location> routes;

};

// class parserFile {
//     //Auto increment counter
//     uint32_t numfile; 

//     //Vector of priorities ids
//     std::vector<uint32_t> prioIdServ;
    
//     //Server vector
//     std::vector<Server> serverDefinitinos;
// };




#endif