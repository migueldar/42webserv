#include "webserv.hpp"


bool isRegularFile(const std::string& filePath) {
    struct stat fileInfo;

    if (stat(filePath.c_str(), &fileInfo) != 0) {
        return false;
    }

    return S_ISREG(fileInfo.st_mode);
}


ParserFile::ParserFile(std::string routeToParserFile): configParserFile(routeToParserFile == "" ? DEFAULT_CONFIG_ParserFile : routeToParserFile.c_str()) {
    if(!isRegularFile(routeToParserFile)){
        throw std::runtime_error("Error: Bad file");
    }

    if (!configParserFile.is_open()) {
        throw std::runtime_error("Error: ParserFile couldn't be opened: " + routeToParserFile);
    }

    fillServers();

    if(serverDefinitions.size() == 0)
        throw std::runtime_error("Error: no servers defined");

    for(std::map<unsigned long, std::vector<Server> >::iterator it = serverDefinitions.begin(); it != serverDefinitions.end(); ++it)
        printServersByPort(it->first);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void removeLastSemicolon(std::string& word) {
    if(!word.empty() && word[word.length() - 1] == ';')
        word.erase(word.length() - 1);
}


// En el archivo CDParserFile.cpp
void ParserFile::printServersByPort(unsigned long targetPort) {
    std::map<unsigned long, std::vector<Server> >::iterator serverIter = serverDefinitions.find(targetPort);

    if (serverIter != serverDefinitions.end()) {
        std::cout << "Servers for port " << targetPort << ":" << std::endl;

        std::vector<Server>& servers = serverIter->second;
        for (std::vector<Server>::iterator serverVecIter = servers.begin(); serverVecIter != servers.end(); ++serverVecIter) {
            const Server& server = *serverVecIter;

            std::cout << "  Server Name: " << server.serverName << std::endl;

            std::cout << "  Error Pages:" << std::endl;
            std::map<std::string, std::string>::const_iterator errorPageIter = server.getErrPages().begin();
            while (errorPageIter != server.getErrPages().end()) {
                std::cout << "    Status: " << errorPageIter->first << ", Route: " << errorPageIter->second << std::endl;
                ++errorPageIter;
            }

            std::cout << "  Locations:" << std::endl;
            const std::map<std::string, Location>& routes = server.getRoutes();
            for (std::map<std::string, Location>::const_iterator locationIter = routes.begin(); locationIter != routes.end(); ++locationIter) {
                const std::string& path = locationIter->first;
                const Location& location = locationIter->second;

                std::cout << "    Location Path: " << path << std::endl;
                std::cout << "      Root: " << location.root << std::endl;
                std::cout << "      Default Path: " << location.defaultPath << std::endl;
                std::cout << "      Redirection URL: " << location.redirectionUrl << std::endl;
                std::cout << "      Methods: ";
                for (int i = 0; i < METHODS_NUM; ++i) {
                    std::cout << (location.methods[i] ? "1" : "0") << " ";
                }
                std::cout << std::endl;
                std::cout << "      Autoindex: " << (location.autoindex ? "true" : "false") << std::endl;
            }
        }
    } else {
        std::cout << "No servers found for port " << targetPort << std::endl;
    }
}


int ParserFile::checkRoutesServer(const std::map<std::string, Location>& routes, const std::string& keyToFind) {
    std::map<std::string, Location>::const_iterator iter = routes.find(keyToFind);

    if (iter != routes.end()) {
        return 1;
    } else {
        return 0;
    }
}

std::vector<std::string> ParserFile::getRoutesKeysByPort(unsigned long port) const {
    std::vector<std::string> routes;

    std::map<unsigned long, std::vector<Server> >::const_iterator it = serverDefinitions.find(port);
    if (it != serverDefinitions.end()) {
        const std::vector<Server>& servers = it->second;
        for (std::vector<Server>::const_iterator serverIter = servers.begin(); serverIter != servers.end(); ++serverIter) {
            const std::vector<std::string>& serverRoutes = serverIter->getKeysRoutes();

            for (std::vector<std::string>::const_iterator routeIter = serverRoutes.begin(); routeIter != serverRoutes.end(); ++routeIter) {
                routes.push_back(*routeIter);
            }
        }
    }

    return routes;
}


bool checkAllRoutesByServerVec(const std::vector<Server>& servers, const std::vector<std::string>& newRoutes) {
    for (std::vector<Server>::const_iterator serverIter = servers.begin(); serverIter != servers.end(); ++serverIter) {
        std::vector<std::string> serverRoutes = serverIter->getKeysRoutes();

        for (std::vector<std::string>::const_iterator routeIter = newRoutes.begin(); routeIter != newRoutes.end(); ++routeIter) {
            if (std::find(serverRoutes.begin(), serverRoutes.end(), *routeIter) != serverRoutes.end()) {
                return true;
            }
        }
    }
    return false;
}

void parseMethods(std::vector<std::string> wordLines, int lineNum, bool *aux){

    for (unsigned long i = 1; i < wordLines.size(); i++)
    {
        if (wordLines[i] == "GET")
            aux[GET] = 1;
        else if(wordLines[i] == "POST")
            aux[POST] = 1;
        else if(wordLines[i] == "DELETE")
            aux[DELETE] = 1;
        else
            throw std::runtime_error("Error line " + toString(lineNum) + ": unknown method:" + wordLines[i]);
    }
}

bool isClientErrorHttpCode(const std::string& httpCode) {
    if (httpCode.size() != 3) {
        return false;
    }

    for (std::string::const_iterator it = httpCode.begin(); it != httpCode.end(); ++it) {
        if (!std::isdigit(*it)) {
            return false;
        }
    }

    std::istringstream iss(httpCode);
    int errorCode;
    iss >> errorCode;

    return ((errorCode >= 400 && errorCode < 417) || errorCode == 426 || errorCode == 451);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned long parsePort(std::string portString){
    unsigned long port;
    bool flag = 0;
    
    if(portString[0] == '-')
        flag = 1;
    try{
        port = stringToUnsignedLong(portString);

        if(port == 0)
            flag = 1; 
    }
    catch(std::invalid_argument &e){
        flag = 1;
    }
    
    if(flag == 1)
        throw std::runtime_error("Error line: bad config port: " + portString); 

    return port;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


static ConfigType stringToConfigType(const std::string& str, const std::map<std::string, ConfigType> &configTypeMap) {
    std::map<std::string, ConfigType>::const_iterator it = configTypeMap.find(str);
    return (it != configTypeMap.end()) ? it->second : UNKNOWN;
}

static void setValuesConfigTypeMap(std::map<std::string, ConfigType> &configTypeMap, bool hard = 0){
    if(configTypeMap["server"] != UNKNOWN || hard == 1)
        configTypeMap["server"] = SERVER;
    if(configTypeMap["server_name"] != UNKNOWN || hard == 1)
        configTypeMap["server_name"] = SERVER_NAME;
    configTypeMap["location"] = LOCATION;
    configTypeMap["error_page"] = ERROR_PAGE;
    if(configTypeMap["port"] != UNKNOWN || hard == 1)
        configTypeMap["port"] = PORT;
    configTypeMap["}"] = BRACE_CLOSE;
}

static void setValuesLocation(std::map<std::string, ConfigType> &configTypeMap){
    configTypeMap["redirect"] = REDIRECT;
    configTypeMap["root"] = ROOT;
    configTypeMap["index"] = INDEX;
    configTypeMap["methods"] = METHODS;
    configTypeMap["auto_index"] = AUTO_INDEX;
    configTypeMap["location"] = UNKNOWN;
    configTypeMap["error_page"] = UNKNOWN;
    configTypeMap["}"] = BRACE_CLOSE;
}

void ParserFile::fillServers() {
    long brace = 0;
    unsigned int port;
    Location location;
    std::string routeKey;
    std::string line;
    std::map<std::string, ConfigType> configTypeMap;
    std::vector<std::string> wordLines;
    std::vector<std::string>::iterator it;
    Server server;
    unsigned int lineNum = 0;

    setValuesConfigTypeMap(configTypeMap);

    while (std::getline(configParserFile, line)) {
        ++lineNum;
        wordLines = splitString(line, ' ');

        if (wordLines.empty() || (wordLines[0])[0] == '#'){
            continue;
        }

        removeLastSemicolon(wordLines[wordLines.size() - 1]);

        if(wordLines[wordLines.size() - 1] == "")
            throw std::runtime_error("Error line " + toString(lineNum) + ": bad config no input: ';'");

        ConfigType configType = stringToConfigType(wordLines[0], configTypeMap);

        switch (configType) {
            case SERVER:
                if(wordLines.size() == 2 && wordLines[1] == "{" && brace == 0){
                    server = Server();
                    ++brace;
                }
                else
                    throw std::runtime_error("Error line " + toString(lineNum) + ": bad config server:" + *(--wordLines.end()));
                break;

            //(server_name)
            case SERVER_NAME:
                configTypeMap["server_name"] = UNKNOWN;
                if(wordLines.size() == 2 && brace == 1){
                    server.serverName = wordLines[1];
                }
                else
                    throw std::runtime_error("Error line " + toString(lineNum) + ": bad config server_name:" + *(--wordLines.end()));
                break;

            //(port)
            case PORT:
                configTypeMap["port"] = UNKNOWN;
                if(wordLines.size() == 2){
                    port = parsePort(wordLines[1]);
                }
                else
                    throw std::runtime_error("Error line " + toString(lineNum) + ": bad config port:" + *(--wordLines.end()));
                break;

            //(location)    
            case LOCATION:
                if(wordLines.size() == 3 && wordLines[2] == "{" && brace == 1){
                    setValuesLocation(configTypeMap);
                    location = Location();
                    if(wordLines[1][0] != '/')
                        throw std::runtime_error("Error line " + toString(lineNum) + ": not valid location dir:" + wordLines[1]);
                    if(checkRoutesServer(server.getRoutes(), wordLines[1]) == 1)
                        throw std::runtime_error("Error line " + toString(lineNum) + ": duplicated location dir:" + wordLines[1]);
                    routeKey = wordLines[1];
                    ++brace;
                }
                else
                    throw std::runtime_error("Error line " + toString(lineNum) + ": bad config location:" + *(--wordLines.end()));
                break;

            //(redirect)    
            case REDIRECT:
                configTypeMap["redirect"] = UNKNOWN;
                if(wordLines.size() == 2 && brace == 2){
                    if(wordLines[1][0] != '/')
                        throw std::runtime_error("Error line " + toString(lineNum) + ": not valid redirect dir:" + wordLines[1]);

                    location.redirectionUrl = wordLines[1];
                }
                else
                    throw std::runtime_error("Error line " + toString(lineNum) + ": bad config redirection:" + *(--wordLines.end()));
                break;
            
            //(error_page)
            case ERROR_PAGE:
                if(wordLines.size() == 3 && brace == 1){
                    if(!isClientErrorHttpCode(wordLines[1]))
                        throw std::runtime_error("Error line " + toString(lineNum) + ": error_page status code error: " + wordLines[1]);

                    if(server.getPageStatus(wordLines[1]) != "")
                        throw std::runtime_error("Error line " + toString(lineNum) + ": error_page status code already used: " + wordLines[1]);

                    server.addErrorPage(wordLines[1], wordLines[2]);
                }
                else
                    throw std::runtime_error("Error line " + toString(lineNum) + ": bad config error_page:" + *(--wordLines.end()));
                break;
            
            //(root)
            case ROOT:
                configTypeMap["root"] = UNKNOWN;
                if(wordLines.size() == 2 && brace == 2){
                    if(wordLines[1][0] != '/')
                        throw std::runtime_error("Error line " + toString(lineNum) + ": not valid root dir:" + wordLines[1]);

                    location.root = wordLines[1];
                }
                else
                    throw std::runtime_error("Error line " + toString(lineNum) + ": bad config root:" + *(--wordLines.end()));
                break;

            //(index)
            case INDEX:
                configTypeMap["index"] = UNKNOWN;
                if(wordLines.size() == 2 && brace == 2){
                    location.defaultPath = wordLines[1];
                }
                else
                    throw std::runtime_error("Error line " + toString(lineNum) + ": bad config index:" + *(--wordLines.end()));
                break;

            //(auto_index)
            case AUTO_INDEX:
                configTypeMap["auto_index"] = UNKNOWN;
                if(wordLines.size() == 2 && brace == 2){
                    if(wordLines[1] == "true")
                        location.autoindex = 1;
                    else if(wordLines[1] == "false")
                        location.autoindex = 0;
                    else 
                        throw std::runtime_error("Error line " + toString(lineNum) + ": auto_index not valid config" + wordLines[1]);
                }
                else
                    throw std::runtime_error("Error line " + toString(lineNum) + ": bad config auto_index:" + *(--wordLines.end()));
                break;
            
            //(methods)
            case METHODS:
                configTypeMap["methods"] = UNKNOWN;
                if(wordLines.size() >= 2 && wordLines.size() <= 4 && brace == 2){
                    bool aux[3] = {0};
                    parseMethods(wordLines, lineNum, aux);
                    if(aux[GET] == 0)
                        location.methods[GET] = 0;
                    if(aux[POST] == 0)
                        location.methods[POST] = 0;
                    if(aux[POST] == 0)
                        location.methods[DELETE] = 0;
                }
                else
                    throw std::runtime_error("Error line " + toString(lineNum) + ": bad config methods:" + *(--wordLines.end()));
                break;

            //(})   
            case BRACE_CLOSE:
                if (wordLines[0] == "}" && wordLines.size() == 1){
                    --brace;
                    if(brace == 0){
                        if(configTypeMap["port"] != UNKNOWN){
                            throw std::runtime_error("Error line " + toString(lineNum) + ": bad config, missing port");
                        }
                        if(!server.getNumRoutes()){
                            throw std::runtime_error("Error line " + toString(lineNum) + ": bad config, missing location");
                        }
                        if(checkAllRoutesByServerVec(serverDefinitions[port], server.getKeysRoutes()) == 1){
                            throw std::runtime_error("Error line " + toString(lineNum) + ": one route or more is already registered to this port");
                        }
                        serverDefinitions[port].insert(serverDefinitions[port].begin(), server);
                        setValuesConfigTypeMap(configTypeMap, 1);
                    }
                    if(brace == 1){
                        if(configTypeMap["index"] != UNKNOWN && configTypeMap["root"] != UNKNOWN && configTypeMap["redirection"] != UNKNOWN)
                            throw std::runtime_error("Error line " + toString(lineNum) + ": invalid location");
                        server.addLocation(routeKey, location);
                        setValuesConfigTypeMap(configTypeMap);
                    }
                }
                else
                    throw std::runtime_error("Error line " + toString(lineNum) + ": bad config: " + wordLines[0]);
                break;
            default:
                throw std::runtime_error("Error line " + toString(lineNum) + ": bad config: " + wordLines[0]);
            break;
        }
        if(brace == 0 && configType != BRACE_CLOSE){
            throw std::runtime_error("Error line " + toString(lineNum) + ": bad config: " + wordLines[0]);
        }            
    }
}



ParserFile::~ParserFile() {
    configParserFile.close();
}
