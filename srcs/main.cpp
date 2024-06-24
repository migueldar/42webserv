#include "webserv.hpp"

std::vector<Listener> createListeners(const ParserFile& file) {
	std::vector<Listener>								ret;
	const std::map<unsigned int, std::vector<Server> >	servers = file.getServers();

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

int main(int argc, char **argv) {
	std::string				configRoute = "";
	ParserFile				config;

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

	std::vector<Listener>	listeners = createListeners(config);
	for (unsigned long i = 0; i < listeners.size(); i++) {
		std::cout << listeners[i].port << std::endl;
	}
	PollHandler	po = createPollHandler(listeners);
	// while (1)
	// 	po.pollMode();
	return 0;
}
