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
        throw std::runtime_error("Error: bad config port: " + portString); 

    return port;
}

//Location ParserFile::parseLocation(){}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


static ConfigType stringToConfigType(const std::string& str, const std::map<std::string, ConfigType> &configTypeMap) {
    std::map<std::string, ConfigType>::const_iterator it = configTypeMap.find(str);
    return (it != configTypeMap.end()) ? it->second : UNKNOWN;
}

static void serValuesConfigTypeMap(std::map<std::string, ConfigType> &configTypeMap){
    configTypeMap["server"] = SERVER;
    configTypeMap["server_name"] = SERVER_NAME;
    configTypeMap["location"] = LOCATION;
    configTypeMap["redirect"] = REDIRECT;
    configTypeMap["error_page"] = ERROR_PAGE;
    configTypeMap["port"] = PORT;
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

    serValuesConfigTypeMap(configTypeMap);

    while (std::getline(configParserFile, line)) {
        wordLines = splitString(line, ' ');
        it = wordLines.begin();

        if (wordLines.empty() || (*it)[0] == '#'){
            continue;
        }

        ConfigType configType = stringToConfigType(*it, configTypeMap);

        switch (configType) {
            case SERVER:
                if(wordLines.size() == 2 && wordLines[1] == "{"){
                    serValuesConfigTypeMap(configTypeMap);
                    tmp = Server();
                    ++brace;
                    std::cout << "SERVER FOUND"<< std::endl;
                }
                else
                    throw std::runtime_error("Error: bad config server:" + *(--wordLines.end()));
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
                    throw std::runtime_error("Error: bad config server_name:" + *(--wordLines.end()));
                break;

            //(port)
            case PORT:
                configTypeMap["port"] = UNKNOWN;
                if(wordLines.size() == 2){
                    port = parsePort(wordLines[1]);
                    
                    std::cout << "PORT FOUND:" << toString(port) << std::endl;
                }
                else
                    throw std::runtime_error("Error: bad config port:" + *(--wordLines.end()));
                break;

            //(location)    
            case LOCATION:
                if(wordLines.size() == 3 && wordLines[2] == "{"){
                    location = Location();
                    routeKey = wordLines[1];
                    std::cout << "LOCATION FOUND: key:" << routeKey << std::endl;
                    ++brace;
                }
                else
                    throw std::runtime_error("Error: bad config location:" + *(--wordLines.end()));
                break;

            //(redirect)    
            case REDIRECT:
                configTypeMap["redirect"] = UNKNOWN;
                std::cout << "REDIRECT FOUND" << std::endl;
                break;
            
            //(error_page)
            case ERROR_PAGE:
                std::cout << "ERROR PAGE FOUND" << std::endl;
                break;
            
            //(root)
            case ROOT:
                configTypeMap["root"] = UNKNOWN;
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
                            throw std::runtime_error("Error: bad config, missing port");
                        }
                        if(tmp.getNumRoutes() == 0){
                            throw std::runtime_error("Error: bad config, missing location");
                        }

                        serverDefinitions[port].insert(serverDefinitions[port].begin(), tmp);
                    }
                    if(brace == 1){
                        
                        tmp.addLocation(routeKey, location);
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
            throw std::runtime_error("Error: bad config: " + *it);
        }
        if (flag == 1)
            throw std::runtime_error("Error: bad config: " + *it);
    }
}



ParserFile::~ParserFile() {
    configParserFile.close();
}
