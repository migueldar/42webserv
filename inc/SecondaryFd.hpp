#ifndef SECONDARYFD_HPP
#define SECONDARYFD_HPP

class SecondaryFd {
	public:
		int	fd;
		//read 0, write 1
		int	rw;
		bool operator==(const SecondaryFd &other) const {
			return (fd == other.fd) && (rw == other.rw);
		}
		SecondaryFd& operator=(SecondaryFd const& rhs) {
			fd = rhs.fd;
			rw = rhs.rw;
			return *this;
		}
};

#endif