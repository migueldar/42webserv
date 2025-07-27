
#include "webserv.hpp"

CgiHandler::CgiHandler(const Location &loc, std::string &tokenCGI, std::string &port, Request &req, std::vector<std::string> &uri, std::string &query_string): tokenCGI(tokenCGI), port(port), req(req), uri(uri), query_string(query_string), loc(loc), stages(BEGIN_CGI_EXEC), pid(0), hasBeenWaited(false){
    initDictParser();
    for (enum metaVariables x = LOCATION; x < METAVARIABLES_LENGTH; x = static_cast<enum metaVariables>(x + 1)) {
        (this->*methodMap[x])();
    }
    for (std::map<std::string, std::string>::iterator it = metaVariables.begin(); it != metaVariables.end(); it++){
        // std::cout << it->first << "=" << it->second << std::endl;
    }

	reqbody = req.body;
    script = loc.root + script;
}

CgiHandler::~CgiHandler() {
}

CgiHandler::CgiHandler(const CgiHandler& other)
    : tokenCGI(other.tokenCGI), port(other.port), req(other.req), uri(other.uri), query_string(other.query_string), loc(other.loc), env(NULL), stages(other.stages), pid(other.pid), hasBeenWaited(other.hasBeenWaited) {
}

long CgiHandler::handleCgiEvent(int err) {
	char*	aux;
	int		lenToWrite;
	enum CGI_STAGES auxStage;

	if(err){
		auxStage = stages;
		stages = WAITPID_CGI;
	}

    switch (stages) {
        case BEGIN_CGI_EXEC:
            if (pipe(infd) < 0 || pipe(outfd) < 0) {
                std::cerr << "Error al crear las tuberías: " << strerror(errno) << std::endl;
                return -1;
            }
            pid = fork();
            if (pid < 0) {
                std::cerr << "Error al crear el proceso hijo: " << strerror(errno) << std::endl;
                return -1;
            }
            if (pid == 0) {
                close(infd[1]);
                close(outfd[0]);
                dup2(infd[0], STDIN_FILENO); 
                dup2(outfd[1], STDOUT_FILENO);

                size_t numVariables = metaVariables.size();
    
                env = new char *[numVariables + 1];

                size_t i = 0;
                
                for (std::map<std::string, std::string>::const_iterator iter= metaVariables.begin(); iter != metaVariables.end(); ++iter) {
                    std::string envVar = iter->first + "=" + iter->second;

                    env[i] = new char[envVar.length() + 1]; 
                    strcpy(env[i], envVar.c_str()); 
                    ++i;
                }

                env[numVariables] = NULL;

                const char* args[] = { (loc.cgi.at(tokenCGI)).c_str(), script.c_str(), NULL };
                if (execve(args[0], (char* const*)args, (char* const*)env) < 0) {
                    std::cerr << "Error al ejecutar el script CGI: " << strerror(errno) << std::endl;
                    for (unsigned long i = 0; i < metaVariables.size(); i++)
                        delete[] env[i];
                    delete[] env;
                    exit(EXIT_FAILURE);
                }
            } else {
                close(infd[0]);
                close(outfd[1]);
                stages = WRITE_CGI_EXEC;
                long ret = infd[1] | ((long)1 << 32);
                return ret;
            }
            break;
        case WRITE_CGI_EXEC:
			// std::cout << "writting pipe: " << infd[1] << std::endl;

			aux = reqbody.popFirst(lenToWrite);
            if (write(infd[1], aux, lenToWrite) < 0) {
                close(infd[1]);
                std::cerr << "Error al escribir en el pipe: " << strerror(errno) << std::endl;
				delete[] aux;
                return -1;
            }
			delete[] aux;
            if (reqbody.empty()) {
				close(infd[1]);
            	stages = READ_CGI_EXEC;
            	return (long)outfd[0];
			}
			{
				long ret = infd[1] | ((long)1 << 32);
            	return ret;
			}
			break;
        case READ_CGI_EXEC:
			char read_buff[SIZE_READ + 1];
			memset(read_buff, 0, SIZE_READ + 1);

			if (read(outfd[0], read_buff, SIZE_READ) >= 0) {
                response += read_buff;
				// std::cout << "read till now len:" << response.length() << std::endl;
			}
			else {
				perror("Error del script CGI");
				close(outfd[0]);
                return -1;
			}
			{
				size_t index = response.find("EOF");
				if (index != std::string::npos && index == response.length() - 3){
					response = response.subdeque(0, index);
					close(outfd[0]);
				}
				else {
					return (long)outfd[0];
				}
			}
            // fall through
		case WAITPID_CGI:
			if (!hasBeenWaited) {
				hasBeenWaited = true;
				int status;
				waitpid(pid, &status, 0);
                if (WEXITSTATUS(status) != EXIT_SUCCESS){
                    close(outfd[0]);
                    std::cerr << "El proceso hijo terminó sin éxito: " << WEXITSTATUS(status) << std::endl;
                    return -1;
                }
				if(stages == WAITPID_CGI){
					stages = auxStage;
					return (long)outfd[0];
				}
			}
			break;
    }
    return 0;
}


stringWrap CgiHandler::getCgiResponse() const {
    return response;
}

void CgiHandler::initDictParser(void){
    methodMap[LOCATION] = &CgiHandler::parseLOCATION;
    methodMap[SCRIPT_NAME] = &CgiHandler::parseSCRIPT_NAME;
    methodMap[PATH_INFO] = &CgiHandler::parsePATH_INFO;
    methodMap[PATH_TRANSLATED] = &CgiHandler::parsePATH_TRANSLATED;
    methodMap[QUERY_STRING] = &CgiHandler::parseQUERY_STRING;
    methodMap[REQUEST_METHOD] = &CgiHandler::parseREQUEST_METHOD;
    methodMap[SERVER_PROTOCOL] = &CgiHandler::parseSERVER_PROTOCOL;
    methodMap[SERVER_NAME] = &CgiHandler::parseSERVER_NAME;
    methodMap[REMOTE_ADDR] = &CgiHandler::parseREMOTE_ADDR;
    methodMap[SERVER_PORT] = &CgiHandler::parseSERVER_PORT;
    methodMap[SERVER_SOFTWARE] = &CgiHandler::parseSERVER_SOFTWARE;
    methodMap[AUTH_TYPE] = &CgiHandler::parseAUTH_TYPE;
    methodMap[CONTENT_LENGTH] = &CgiHandler::parseCONTENT_LENGTH;
    methodMap[CONTENT_TYPE] = &CgiHandler::parseCONTENT_TYPE;
    methodMap[GATEWAY_INTERFACE] = &CgiHandler::parseGATEWAY_INTERFACE;
}

void CgiHandler::parseLOCATION(void) {
    std::string location = "";

    while (!uri.empty()) {
        size_t found = uri[0].find(tokenCGI);
        if (found != std::string::npos) {
            metaVariables["LOCATION"] = location;
            break;
        }
        location += "/" + uri[0];
        uri.erase(uri.begin());
    }
    if(location == ""){
        metaVariables["LOCATION"] = "/";
    }
    return;
}

void CgiHandler::parseSCRIPT_NAME(void) {
    bool    bad = 0;

    for (size_t i = 0; i < uri.size(); i++) {
        size_t found = uri[i].find(tokenCGI);

        if (found != std::string::npos) {
            bad += 1;
            script = uri[i];
            
            metaVariables["SCRIPT_NAME"] = metaVariables["LOCATION"] + "/" + script;
            uri.erase(uri.begin());
            if(!uri.empty()){
                metaVariables["PATH_INFO"] = "/" + script;
            }
        }
    }
    if(bad != 1){
        //throw MAS DE UN SCRIPT EN URL
        return;
    }
    return;
}

void	CgiHandler::parsePATH_INFO(void){
    std::string pathinfo = "";

    while (!uri.empty()) {
        pathinfo += uri[0];
        uri.erase(uri.begin());
    }
    metaVariables["PATH_TRANSLATED"] += pathinfo;
    return;
}

void	CgiHandler::parsePATH_TRANSLATED(void){
    if(metaVariables["PATH_TRANSLATED"] != "")
        metaVariables["PATH_TRANSLATED"] = loc.root + metaVariables["PATH_TRANSLATED"];
    return;
}

void	CgiHandler::parseQUERY_STRING(void){
    metaVariables["QUERY_STRING"] = query_string;
    return;
}

void	CgiHandler::parseREQUEST_METHOD(void){
    switch (req.method)
    {
    case GET:
        metaVariables["REQUEST_METHOD"] = "GET";
        break;
    case DELETE:
        metaVariables["REQUEST_METHOD"] = "DELETE";
        break;
    case POST:
        metaVariables["REQUEST_METHOD"] = "POST";
        break;
    }
    return;
}

void	CgiHandler::parseSERVER_PROTOCOL(void){
    metaVariables["SERVER_PROTOCOL"] = "HTTP";
    return;
}

void	CgiHandler::parseSERVER_NAME(void){
    metaVariables["SERVER_NAME"] = req.headers["Host"];
    return;
}

void    CgiHandler::parseREMOTE_ADDR(void){
    // metaVariables["REMOTE_ADDR"] = req.address; TODO REGISTER addrss
    return;
}

void    CgiHandler::parseSERVER_PORT(void){
    metaVariables["SERVER_PORT"] = port;
    return;
}

void	CgiHandler::parseSERVER_SOFTWARE(void){
    metaVariables["SERVER_SOFTWARE"] = "1.1";
    return;
}

void	CgiHandler::parseAUTH_TYPE(void){
    if(req.headers["auth-scheme"] != ""){
        metaVariables["AUTH_TYPE"] = req.headers["auth-scheme"];
    }
    return;
}

void	CgiHandler::parseCONTENT_LENGTH(void){
    if(!req.body.empty()){
        metaVariables["CONTENT_LENGTH"] = toString(req.body.length());
    }
    return;
}

void	CgiHandler::parseCONTENT_TYPE(void){
    if(req.headers["Content-Type"] != ""){
        metaVariables["CONTENT_TYPE"] = req.headers["Content-Type"];
    }
    return;
}

void	CgiHandler::parseGATEWAY_INTERFACE(void){
    metaVariables["GATEWAY_INTERFACE"] = "CGI/1.1";
    return;
}

void CgiHandler::parseREMOTE_USER(void){
    if(metaVariables["AUTH_TYPE"] != ""){
        metaVariables["REMOTE_USER"] = req.headers["Host"];
    }
    return;
}
