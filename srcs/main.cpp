#include "webserv.hpp"

std::vector<Listener> createListeners(const ParserFile& file) {
	std::vector<Listener>								ret;
	const std::map<unsigned int, std::vector<Server> >&	servers = file.getServers();

	for (std::map<unsigned int, std::vector<Server> >::const_iterator i = servers.begin(); i != servers.end(); i++)
		ret.push_back(Listener(i->first, i->second));
	return ret;
}

PollHandler	createPollHandler(std::vector<Listener>& listeners) {
	PollHandler	ret;

	for (std::vector<Listener>::iterator i = listeners.begin(); i != listeners.end(); i++)
		ret.addListener(*i);
	return ret;
}

void signalHandler(int a) {
	(void) a;
	std::cout << std::endl << "Exiting webserv" << std::endl;
	exit(0);
}

int main(int argc, char **argv) {
	std::string				configRoute = "";
	ParserFile				config;

	signal(SIGINT, signalHandler);
	if (argc > 2)
		std::cerr << "Usage: ./webserv [configuration ParserFile]" << std::endl;
	if (argc == 2)
		configRoute = argv[1];
	try {
		config = ParserFile(configRoute);
		
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}

	#if ONLY_PARSING_CONF == 0
		try {
			std::vector<Listener>	listeners = createListeners(config);
			PollHandler				po = createPollHandler(listeners);
			po.listenMode();
			while (1)
				po.pollMode();
		}
		catch (std::exception &e) {
			std::cerr << e.what() << std::endl;
			return 1;
		}
	#endif
	return 0;
}
