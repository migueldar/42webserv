
#include "webserv.hpp"


Location::Location() : autoindex(false) {
    methods[GET] = 1;
    methods[POST] = 1;
    methods[DELETE] = 1;
}