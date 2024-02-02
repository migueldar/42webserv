#include "webserv.hpp"

Connection::~Connection() {}

Connection::Connection(int socket, Server& serv): socket(socket), server(serv) {}