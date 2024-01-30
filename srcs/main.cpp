#include "webserv.hpp"

int main(int argc, char **argv) {
	(void) argv;
	if (argc > 2)
		std::cerr << "Usage: ./webserv [configuration file]" << std::endl;

	return 0;
}
