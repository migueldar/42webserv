#include "webserv.hpp"

ParserFile::ParserFile(std::string routeToParserFile): configParserFile(routeToParserFile == "" ? DEFAULT_CONFIG_ParserFile : routeToParserFile.c_str()) {
    if (!configParserFile.is_open()) {
        throw std::runtime_error("Error: ParserFile couldn't be opened: " + routeToParserFile);
    }

    fillServers();

    for(std::map<unsigned long, std::vector<Server> >::iterator it = serverDefinitions.begin(); it != serverDefinitions.end(); ++it)
        printServersByPort(it->first);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void ParserFile::printServersByPort(unsigned long targetPort) {
    std::map<unsigned long, std::vector<Server> >::iterator serverIter = serverDefinitions.find(targetPort);

    if (serverIter != serverDefinitions.end()) {
        std::cout << "Servers for port " << targetPort << ":" << std::endl;

        std::vector<Server>& servers = serverIter->second;
        std::vector<Server>::iterator serverVecIter = servers.begin();
        while (serverVecIter != servers.end()) {
            const Server& server = *serverVecIter;

            std::cout << "  Server Name: " << server.serverName << std::endl;
            // std::map<std::string, Location>::const_iterator locationIter = server.routes.begin();
            // while (locationIter != server.routes.end()) {
            //     const std::string& path = locationIter->first;
            //     const Location& location = locationIter->second;

            //     std::cout << "    Location Path: " << path << std::endl;

            //     ++locationIter;
            // }

            ++serverVecIter;
        }
    } else {
        std::cout << "No servers found for port " << targetPort << std::endl;
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned long parsePort(std::string portString){
    unsigned long port;
    bool flag = 0;
    
    if(portString[0] == '-')
        flag = 1;
    if(portString[portString.length() - 1] == ';')
        portString.erase(portString.length() - 1);
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
    configTypeMap["}"] = BRACE_CLOSE;
}

void ParserFile::fillServers() {
    long brace = 0;
    bool flag = 0;
    unsigned int port;
    Location location;
    std::string routeKey;
    std::string line;
    std::map<std::string, ConfigType> configTypeMap;
    std::vector<std::string> wordLines;
    std::vector<std::string>::iterator it;
    Server tmp;
    unsigned int lineNum = 0;

    setValuesConfigTypeMap(configTypeMap);

    while (std::getline(configParserFile, line)) {
        ++lineNum;
        wordLines = splitString(line, ' ');
        it = wordLines.begin();

        if (wordLines.empty() || (*it)[0] == '#'){
            continue;
        }

        ConfigType configType = stringToConfigType(*it, configTypeMap);

        switch (configType) {
            case SERVER:
                if(wordLines.size() == 2 && wordLines[1] == "{"){
                    if(brace != 0)
                        throw std::runtime_error("Error line " + toString(lineNum) + ": missing terminator");
                    tmp = Server();
                    ++brace;
                    std::cout << "SERVER FOUND"<< std::endl;
                }
                else
                    throw std::runtime_error("Error line " + toString(lineNum) + ": bad config server:" + *(--wordLines.end()));
                break;

            //(server_name)
            case SERVER_NAME:
                configTypeMap["server_name"] = UNKNOWN;
                if(wordLines.size() == 2){

                    if(wordLines[1][wordLines[1].length() - 1] == ';')
                        wordLines[1].erase(wordLines[1].length() - 1);

                    tmp.serverName = wordLines[1];
                    std::cout << "SERVER NAME FOUND: " + wordLines[1] << std::endl;
                }
                else
                    throw std::runtime_error("Error line " + toString(lineNum) + ": bad config server_name:" + *(--wordLines.end()));
                break;

            //(port)
            case PORT:
                configTypeMap["port"] = UNKNOWN;
                if(wordLines.size() == 2){
                    port = parsePort(wordLines[1]);
                    
                    std::cout << "PORT FOUND:" << toString(port) << std::endl;
                }
                else
                    throw std::runtime_error("Error line " + toString(lineNum) + ": bad config port:" + *(--wordLines.end()));
                break;

            //(location)    
            case LOCATION:
                if(wordLines.size() == 3 && wordLines[2] == "{"){
                    setValuesLocation(configTypeMap);
                    location = Location();
                    routeKey = wordLines[1];
                    std::cout << "LOCATION FOUND: key:" << routeKey << std::endl;
                    ++brace;
                }
                else
                    throw std::runtime_error("Error line " + toString(lineNum) + ": bad config location:" + *(--wordLines.end()));
                break;

            //(redirect)    
            case REDIRECT:
                configTypeMap["redirect"] = UNKNOWN;
                if(wordLines.size() == 2){

                    if(wordLines[1][wordLines[1].length() - 1] == ';')
                        wordLines[1].erase(wordLines[1].length() - 1);

                    location.redirectionUrl = wordLines[1];
                    std::cout << "REDIRECT FOUND" << std::endl;
                }
                else
                    throw std::runtime_error("Error line " + toString(lineNum) + ": bad config redirection:" + *(--wordLines.end()));
                break;
            
            //(error_page)
            case ERROR_PAGE:
                std::cout << "ERROR PAGE FOUND" << std::endl;
                break;
            
            //(root)
            case ROOT:
                configTypeMap["root"] = UNKNOWN;
                if(wordLines.size() == 2){

                    if(wordLines[1][wordLines[1].length() - 1] == ';')
                        wordLines[1].erase(wordLines[1].length() - 1);

                    location.redirectionUrl = wordLines[1];
                    std::cout << "REDIRECT FOUND" << std::endl;
                }
                else
                    throw std::runtime_error("Error line " + toString(lineNum) + ": bad config redirection:" + *(--wordLines.end()));
                break;
                std::cout << "ROOT FOUND" << std::endl;
                break;
            
            //(index)
            case INDEX:
                configTypeMap["index"] = UNKNOWN;
                std::cout << "INDEX FOUND" << std::endl;
                break;
            
            //(methods)
            case METHODS:
                configTypeMap["methods"] = UNKNOWN;
                std::cout << "METHODS FOUND" << std::endl;
                break;

            //(})   
            case BRACE_CLOSE:
                if (++it == wordLines.end()){
                    --brace;
                    if(brace == 0){
                        if(configTypeMap["port"] != UNKNOWN){
                            throw std::runtime_error("Error line " + toString(lineNum) + ": bad config, missing port");
                        }
                        if(!tmp.getNumRoutes()){
                            throw std::runtime_error("Error line " + toString(lineNum) + ": bad config, missing location");
                        }
                        if(checkAllRoutesByServerVec(serverDefinitions[port], tmp.getKeysRoutes()) == 1){
                            throw std::runtime_error("Error line " + toString(lineNum) + ": one route or more is already registered to this port");
                        }

                        serverDefinitions[port].insert(serverDefinitions[port].begin(), tmp);
                        setValuesConfigTypeMap(configTypeMap, 1);
                    }
                    if(brace == 1){
                        
                        tmp.addLocation(routeKey, location);
                        setValuesConfigTypeMap(configTypeMap);
                    }
                }
                else
                    flag = 1;
                break;
            default:
                flag = 1;
            break;
        }
        if(brace == 0 && configType != BRACE_CLOSE){
            throw std::runtime_error("Error line " + toString(lineNum) + ": bad config: " + *it);
        }
        if (flag == 1)
            throw std::runtime_error("Error line " + toString(lineNum) + ": bad config: " + *it);
    }
}



ParserFile::~ParserFile() {
    configParserFile.close();
}
