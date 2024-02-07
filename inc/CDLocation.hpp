#ifndef CDLOCATION_HPP
#define CDLOCATION_HPP

#include <iostream>

#define METHODS_NUM 3

// src/paser/CDLocation.cpp
class Location {
	public:
	    //ATRIBUTES------------------------------------------------------------------·#
		std::string root;
		//NOT DEFAULT NEEDED
		std::string defaultPath;
		// NO DEFAULT NEEDED 
		std::string redirectionUrl;
		//MANAGES ESPECIFIC RULES FOR METHODS
		bool methods[METHODS_NUM];
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