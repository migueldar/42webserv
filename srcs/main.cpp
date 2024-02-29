#include "webserv.hpp"

int main(int argc, char **argv, char **envp) {
	int ret = 0;
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
		std::cout << e.what() << std::endl;
		ret =  1;
	}
	return ret;
}
