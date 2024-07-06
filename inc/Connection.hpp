#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "parser.hpp"
#include "Request.hpp"
#include "Response.hpp"

#define SIZE_READ 0x1000

class Connection {
	private:
		time_t		startTime;
		//only on false when server is doing response processing
		bool		checkTime;

	public:
		//read 0, write 1
		class secondaryFd {
			public:
				int	fd;
				int	rw;
				bool operator==(const secondaryFd &other) const {
					return (fd == other.fd) && (rw == other.rw);
				}
				secondaryFd& operator=(secondaryFd const& rhs) {
					fd = rhs.fd;
					rw = rhs.rw;
					return *this;
				}
		};
		int							port;
		int							sock;
		secondaryFd					secFd;
		const std::vector<Server>	&servers;
		//if set to NULL, no request is being parsed currently
		Request						*req;
		Response 					*res;
		//saves data which hasnt being processed by a request yet
		std::string					data;

		Connection(int port, int socket, const std::vector<Server> &servers);
		Connection(const Connection &other);
		~Connection();
		void startTimer();
		bool checkTimer() const;
		bool operator==(const Connection &other) const;
		int handleEvent(struct pollfd &pollfd);
		//if nothing wants to be returned, just return fd == -1
		secondaryFd	handleSecondaryEvent(struct pollfd &pollfd, int revent);
};

#endif
