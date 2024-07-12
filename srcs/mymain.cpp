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

void testReq(std::string str) {
	std::cout << "------------------------------------------" << std::endl;
	std::cout << "Test:\n" << str << std::endl << std::endl;

	stringWrap wr;
	wr += str; 
	Request a;
	stringWrap b = a.addData(wr);
	std::cout << a;
	std::cout << "============================" << std::endl << "Extra Data: " << std::endl << b << std::endl;
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
		testReq("GET /%20data HTTP/1.1\r\nHost:''\r\n\r\n");
		testReq("POST /%20data/miau/ HTTP/1.9\r\nHost:''\r\n\r\n");
		testReq("POST /%20data/miau/ HTTP/1.9\r\nHost:''\r\n\r\n");
		testReq("POST /%20data/miau/?mifgata=?tum#dre%21 HTTP/1.9\r\nHost:''\r\n\r\n");
		testReq("POST /%20data/miau/a?mifgata=?tum%21 HTTP/1.9\r\nHost:''\r\n\r\n");
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
		std::string reqline = "POST / HTTP/1.1\r\n";
		std::string headersHost = "Host: 127.0.0.1\r\n";
		std::string headersChunked = "Host: 127.0.0.1\r\nTransfer-Encoding: chunked\r\n\r\n";
		std::string reqHost = reqline + headersHost;
		std::string reqChunk = reqline + headersChunked;
		//content-length
		testReq(reqHost + "Content-Length: 10\r\n\r\n" + "q1cwu\r\nelmiguemol\r\n    ilpepnomola\r\n00000\r\n\r\n");
		testReq(reqHost + "Content-Length: 80\r\n\r\n1cwu\r\nelmiguemola\r\n    ilpepnomola\r\n00000\r\n\r\n");

		//chunked
		testReq(reqChunk + "q1cwu\r\nelmiguemol\r\n    ilpepnomola\r\n00000\r\n\r\n");
		testReq(reqChunk + "11wu\r\n\r\nelmiguemoaaaaaa\r\n1\r\njuwu\r\n00000\r\n\r\n");
		testReq(reqChunk + "11wu\r\n\r\nelmiguemoaaaaaa\r\n1\r\nj\r\n00000\r\n\r\n");
		testReq(reqChunk + "1cwu\r\nelmiguemola\r\n    ilpepnomola\r\n00000\r\n\r\n");
		testReq(reqChunk + "0000000000000000000000000\r\n\r\n");
		testReq(reqChunk + "\r\n");
		testReq(reqChunk + "x\r\n1\r\n");
		testReq(reqChunk + "0\r\n11\r\n\r\n");

		//body size


	}
	///////////
}