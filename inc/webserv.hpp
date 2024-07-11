#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <exception>
#include <iostream>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "Connection.hpp"
#include "Listener.hpp"
#include "PollHandler.hpp"
#include "parser.hpp"
#include "http.hpp"
#include "CDCgiHandler.hpp"
#include "SecondaryFd.hpp"
#include "StringWrapper.hpp"


#define ONLY_PARSING_CONF 0
#define CONNECTION_TIMEOUT 60 //in seconds
#define RESPONSE_PROCESSING_TIMEOUT 60 //in seconds
#endif