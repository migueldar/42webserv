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

void leaks() {
	system("leaks webserv");
}

void sH(int a) {
	(void) a;
	exit(0);
}

// int main() {
// 	signal(SIGINT, sH);

// 	std::vector<Server> servs = buildSampleServers();
// 	std::vector<Server> servs2 = buildSampleServers();

// 	Listener l1(8080, servs), l2(8081, servs);
// 	//, l3(8082, servs);
// 	// Connection c1(100, servs), c2(101, servs), c3(102, servs);
// 	PollHandler po;

// 	po.addListener(l1);
// 	po.addListener(l2);
// 	po.listenMode();

// 	while (1) {
// 		po.pollMode();
// 		// usleep(500000);
// 	}

// 	//std::cout << "Done" << std::endl;
// }

void testReq(std::string str) {
	std::cout << "------------------------------------------" << std::endl << std::endl;
	std::cout << "Test: " << str << std::endl;
	try {
		Request a;
		a.addData(str);
	} catch (std::exception &e) {
		std::cout << "Res: " << e.what() << std::endl << std::endl;
	}
}

//test parser first line
//bateria de tests
int main() {
	signal(SIGINT, sH);
	// atexit(leaks);

	//first line
	{
		//general
		testReq("");
		testReq("\r\n");
		testReq("meow");
		{
			std::string s = "test";
			while (s.length() <= 8000)
				s += s;
			s += "\r\n";
			testReq(s);
		}

		//method
		testReq("GET\r\n");
		testReq("KK \r\n");
		testReq("K,K / \r\n");

		//target
		testReq("GET /\r\n");
		testReq("GET 90 \r\n");
		testReq("POST /blablabla# \r\n");
		testReq("POST /bla%4Z \r\n");
		testReq("POST /data/blablabla# \r\n");
		testReq("POST /data/bla%4Z \r\n");

		//version
		testReq("GET / fsdafdjaslk\r\n");
		testReq("GET / HHHH/1.1\r\n");
		testReq("GET / HTTP/a.1\r\n");
		testReq("GET / HTTP/111\r\n");
		testReq("GET / HTTP/1.a\r\n");
		testReq("GET / HTTP/2.1\r\n");
		testReq("GET / HTTP/1.0\r\n");

		//general correcto
		testReq("GET /%20data HTTP/1.1\r\n\r\n");
		testReq("POST /%20data/miau/ HTTP/1.9\r\nHost:''\r\n\r\n");
	}
	////////////

	std::cout << std::endl << std::endl << std::endl << std::endl;

	//fields
	{
		//bad name
		testReq("GET / HTTP/1.1\r\nHOLA\r\n\r\n");
		testReq("GET / HTTP/1.1\r\nK,K: \r\n\r\n");
		testReq("GET / HTTP/1.1\r\nHOLA: jej\r\nTONTO\r\n\r\n");
		testReq("GET / HTTP/1.1\r\nHOLA:jj \r\nK,K: \r\n\r\n");

		//bad value
		testReq("GET / HTTP/1.1\r\nHOLA:\x19\r\n\r\n");
		testReq("GET / HTTP/1.1\r\nHOLA:56\x19 31\r\n\r\n");
		testReq("GET / HTTP/1.1\r\nADIOS:\r\nHOLA:\x19\r\n\r\n");
		testReq("GET / HTTP/1.1\r\nADIOS:\r\nHOLA:56\x19 31\r\n\r\n");

		//bad host
		testReq("GET / HTTP/1.1\r\nContent-Length: 10\r\n\r\n");
		testReq("GET / HTTP/1.1\r\nContent-Length: 10\r\nHost:\r\n\r\n");
		testReq("GET / HTTP/1.1\r\nContent-Length: 10\r\nHost:@\r\n\r\n");
		
		//bad body length
		testReq("GET / HTTP/1.1\r\nContent-Length: x\r\nHost:''\r\n\r\n");
		testReq("GET / HTTP/1.1\r\nContent-Length: 543753872975954325342\r\nHost:''\r\n\r\n");
		testReq("GET / HTTP/1.1\r\nTransfer-Encoding: x\r\nHost:''\r\n\r\n");

		//general correcto
		testReq("GET / HTTP/1.1\r\nContent-Length: x\r\nTransfer-Encoding: chunked\r\nHost:''\r\n\r\n");
		testReq("GET / HTTP/1.1\r\nContent-Length: 10\r\nHost:''\r\n\r\n");
		testReq("GET / HTTP/1.1\r\nHost: ''\r\nData:  	  uwu   	  \r\n\r\n");
		testReq("GET / HTTP/1.1\r\nHost: %20e%20e\r\nData:\r\n\r\n");
		testReq("GET / HTTP/1.1\r\nHost: 127.0.0.1\r\nData:\r\nData: n\r\nData: q\r\n\r\n");

	}
	///////////

	std::cout << std::endl << std::endl << std::endl << std::endl;

	//body
	{

	}
	///////////
}