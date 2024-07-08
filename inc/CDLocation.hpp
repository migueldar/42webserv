#ifndef CDLOCATION_HPP
#define CDLOCATION_HPP

#include <iostream>

#define METHODS_NUM 3

// src/paser/CDLocation.cpp
class Location {
	public:
	    //ATRIBUTES------------------------------------------------------------------·#
		std::string root;
		std::string defaultPath;
		std::string redirectionUrl;
		//MANAGES ESPECIFIC RULES FOR METHODS
		bool methods[METHODS_NUM];

		std::string uploadPath;
		std::map<std::string, std::string> cgi;
		//OFF predeterminado
		bool autoindex;
	
	    //METHODS--------------------------------------------------------------------·#
		Location(std::string root);
		Location(Location const& other);
		Location();
		~Location();
		Location& operator=(Location const& rhs);
};


#endif