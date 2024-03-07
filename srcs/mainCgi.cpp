#include "webserv.hpp"

int main() {
	int ret = 0;

	std::vector<std::string> patata;

	patata.push_back("patataCaliente");
	patata.push_back("gorda");
	patata.push_back("script.py");
	patata.push_back("extapath");
	Location loc("/patata");
	Request req();

    CgiHandler CGI(&loc, &("8080"), &req, &patata, ("arguentosRararos=1)");
	
	return ret;
}
