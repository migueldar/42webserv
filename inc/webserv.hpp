#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <algorithm>
#include <arpa/inet.h>
#include <cstdlib>
#include <errno.h>
#include <iostream>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>

#include "Connection.hpp"
#include "Listener.hpp"
#include "PollHandler.hpp"
#include "parser2.hpp"

#endif