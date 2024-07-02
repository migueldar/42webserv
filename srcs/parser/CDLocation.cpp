#include "webserv.hpp"

Location::Location(): autoindex(false) {
    methods[GET] = 1;
    methods[POST] = 1;
    methods[DELETE] = 1;
}

Location::Location(std::string root): root(root) {}

Location::~Location() {}

Location::Location(const Location& other) {
    *this = other;
}

Location& Location::operator=(Location const& other) {
    if (this != &other) {
        root = other.root;
        defaultPath = other.defaultPath;
        redirectionUrl = other.redirectionUrl;
        methods[GET] = other.methods[GET];
        methods[POST] = other.methods[POST];
        methods[DELETE] = other.methods[DELETE];
        autoindex = other.autoindex;
        cgi = other.cgi; 
    }
    return *this;
}

