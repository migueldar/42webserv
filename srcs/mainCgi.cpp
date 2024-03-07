#include "webserv.hpp"

int main() {
	int ret = 0;

	std::vector<std::string> patata;
	std::string token = ".py";
	std::string port = "8080";
	std::string querystring = "arguentosRararos=1";

	patata.push_back("patataCaliente");
	patata.push_back("gorda");
	patata.push_back("script.py");
	patata.push_back("extapath");
	Location loc("/patata");
	Request req;
	req.headers["Host"] = "calientePerro53";

    CgiHandler CGI = CgiHandler(loc, token , port, req, patata, querystring);

	
	return ret;
}
