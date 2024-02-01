#include "webserv.hpp"

int main(int argc, char **argv) {
	std::string configRoute;

	if (argc > 2)
		std::cerr << "Usage: ./webserv [configuration ParserFile]" << std::endl;

	if(argc == 2)
		configRoute = argv[1];
	else
		configRoute = "";
	try{
		ParserFile mainObj(configRoute);
	}
	catch(std::exception &e){
		e.what();
	}

	return 0;
}
