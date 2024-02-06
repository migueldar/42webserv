#include "webserv.hpp"

Server buildSampleServer() {
	Location a("/Users/mde-arpe/CURSUS/webserv/html/miau");
	Location b("/Users/mde-arpe/CURSUS/webserv/html/data");

	std::vector<Location> locations;
	locations.push_back(a);
	locations.push_back(b);

	std::vector<std::string> paths;
	paths.push_back("/gato");
	paths.push_back("/datos");

	Server ret(paths, locations);
	return ret;
}

Server buildSampleServer2() {
	Location c("/Users/mde-arpe/CURSUS/webserv/html");

	std::vector<Location> locations;
	locations.push_back(c);

	std::vector<std::string> paths;
	paths.push_back("/");

	Server ret(paths, locations);
	return ret;
}

std::vector<Server> buildSampleServers() {
	Server ser = buildSampleServer();
	Server ser2 = buildSampleServer2();
	std::vector<Server> servs;
	servs.push_back(ser);
	servs.push_back(ser2);

	return servs;
}

void sH(int a) {
	(void) a;
	exit(0);
}

int main() {
	signal(SIGINT, sH);

	std::vector<Server> servs = buildSampleServers();
	std::vector<Server> servs2 = buildSampleServers();

	Listener l1(8080, servs), l2(8081, servs);
	//, l3(8082, servs);
	// Connection c1(100, servs), c2(101, servs), c3(102, servs);
	PollHandler po;

	po.addListener(l1);
	po.addListener(l2);
	po.listenMode();

	while (1) {
		po.pollMode();
		// usleep(500000);
	}

	//std::cout << "Done" << std::endl;
}
