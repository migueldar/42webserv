
#include "webserv.hpp"
using namespace std;

bool isRegularFile(const string& filePath) {
    struct stat fileInfo;

    if (stat(filePath.c_str(), &fileInfo) != 0) {
        return false;
    }

    return S_ISREG(fileInfo.st_mode);
}

ParserFile::~ParserFile() {
    configParserFile.close();
}

ParserFile::ParserFile(string routeToParserFile): configParserFile(routeToParserFile == "" ? DEFAULT_CONFIG_ParserFile : routeToParserFile.c_str()) {

    if(routeToParserFile == ""){
        routeToParserFile = DEFAULT_CONFIG_ParserFile;
    }

    if(!isRegularFile(routeToParserFile)){
        throw runtime_error("Error: Bad file, file route:" + routeToParserFile);
    }

    if (!configParserFile.is_open()) {
        throw runtime_error("Error: ParserFile couldn't be opened: " + routeToParserFile);
    }

    fillServers();

    if(serverDefinitions.size() == 0)
        throw runtime_error("Error: no servers defined");

    for(map<unsigned int, vector<Server> >::iterator it = serverDefinitions.begin(); it != serverDefinitions.end(); ++it)
        printServersByPort(it->first);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void removeLastSemicolon(string& word) {
    if(!word.empty() && word[word.length() - 1] == ';')
        word.erase(word.length() - 1);
}


void ParserFile::printServersByPort(unsigned int targetPort) {

       std::map<unsigned int, std::vector<Server> >::iterator serverIter = serverDefinitions.find(targetPort);

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

            cout << "  Locations:" << endl;
            const map<string, Location>& routes = server.getRoutes();
            for (map<string, Location>::const_iterator locationIter = routes.begin(); locationIter != routes.end(); ++locationIter) {
                const string& path = locationIter->first;
                const Location& location = locationIter->second;

                cout << "    Location Path: " << path << endl;
                cout << "      Root: " << location.root << endl;
                cout << "      Default Path: " << location.defaultPath << endl;
                cout << "      Redirection URL: " << location.redirectionUrl << endl;
                cout << "      Methods: ";
                for (int i = 0; i < METHODS_NUM; ++i) {
                    cout << (location.methods[i] ? "1" : "0") << " ";
                }
                cout << endl;
                cout << "      Cgi: \n";
                for (map<string, string>::const_iterator it = (location.cgi).begin(); it != (location.cgi).end() ; ++it) {
                    cout << "            " << it->first << " " << it->second << endl;
                }
                cout << endl;
                cout << "      Autoindex: " << (location.autoindex ? "true" : "false") << endl;
            }
        }
    }
}



int ParserFile::checkRoutesServer(const map<string, Location>& routes, const string& keyToFind) {
    map<string, Location>::const_iterator iter = routes.find(keyToFind);

    if (iter != routes.end()) {
        return 1;
    } else {
        return 0;
    }
}

 bool checkAllRoutesByServerVec(const vector<Server>& servers, const vector<string>& newRoutes) {
     for (vector<Server>::const_iterator serverIter = servers.begin(); serverIter != servers.end(); ++serverIter) {
         vector<string> serverRoutes = serverIter->getKeysRoutes();

         for (vector<string>::const_iterator routeIter = newRoutes.begin(); routeIter != newRoutes.end(); ++routeIter) {
             if (find(serverRoutes.begin(), serverRoutes.end(), *routeIter) != serverRoutes.end()) {
                 return true;
             }
         }
     }
     return false;
 }

void parseMethods(vector<string> wordLines, int lineNum, bool *aux){

    for (unsigned long i = 1; i < wordLines.size(); i++)
    {
        if (wordLines[i] == "GET")
            aux[GET] = 1;
        else if(wordLines[i] == "POST")
            aux[POST] = 1;
        else if(wordLines[i] == "DELETE")
            aux[DELETE] = 1;
        else
            throw runtime_error("Error line " + toString(lineNum) + ": unknown method:" + wordLines[i]);
    }
}

bool isClientErrorHttpCode(const string& httpCode) {
    if (httpCode.size() != 3) {
        return false;
    }

    for (string::const_iterator it = httpCode.begin(); it != httpCode.end(); ++it) {
        if (!isdigit(*it)) {
            return false;
        }
    }

    istringstream iss(httpCode);
    int errorCode;
    iss >> errorCode;

    return ((errorCode >= 400 && errorCode <= 417) || errorCode == 426 || errorCode == 451 || (errorCode >= 500 && errorCode <= 505));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned long parsePort(string portString){
    unsigned long port;
    bool flag = 0;
    
    if(portString[0] == '-')
        flag = 1;
    try{
        port = stringToUnsignedLong(portString);

        if(port == 0)
            flag = 1; 
    }
    catch(invalid_argument &e){
        flag = 1;
    }
    
    if(flag == 1)
        throw runtime_error("Error line: bad config port: " + portString); 

    return port;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


static ConfigType stringToConfigType(const string& str, const map<string, ConfigType> &configTypeMap) {
    map<string, ConfigType>::const_iterator it = configTypeMap.find(str);
    return (it != configTypeMap.end()) ? it->second : UNKNOWN;
}

static void setValuesConfigTypeMap(map<string, ConfigType> &configTypeMap, bool hard = 0){
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

static void setValuesLocation(map<string, ConfigType> &configTypeMap){
    configTypeMap["redirect"] = REDIRECT;
    configTypeMap["root"] = ROOT;
    configTypeMap["index"] = INDEX;
    configTypeMap["methods"] = METHODS;
    configTypeMap["auto_index"] = AUTO_INDEX;
    configTypeMap["location"] = UNKNOWN;
    configTypeMap["error_page"] = UNKNOWN;
    configTypeMap["cgi"] = CGI;
    configTypeMap["}"] = BRACE_CLOSE;
}

void ParserFile::fillServers() {
    long brace = 0;
    unsigned int port;
    Location location;
    string routeKey;
    string line;
    map<string, ConfigType> configTypeMap;
    vector<string> wordLines;
    vector<string>::iterator it;
    Server server;
    unsigned int lineNum = 0;

    setValuesConfigTypeMap(configTypeMap);

    while (getline(configParserFile, line)) {
        ++lineNum;
        wordLines = splitString(line, ' ');

        if (wordLines.empty() || (wordLines[0])[0] == '#'){
            continue;
        }

        removeLastSemicolon(wordLines[wordLines.size() - 1]);

        if(wordLines[wordLines.size() - 1] == "")
            throw runtime_error("Error line " + toString(lineNum) + ": bad config no input: ';'");

        ConfigType configType = stringToConfigType(wordLines[0], configTypeMap);

        switch (configType) {
            case SERVER:
                if(wordLines.size() == 2 && wordLines[1] == "{" && brace == 0){
                    server = Server();
                    ++brace;
                }
                else
                    throw runtime_error("Error line " + toString(lineNum) + ": bad config server:" + *(--wordLines.end()));
                break;

            //(server_name)
            case SERVER_NAME:
                configTypeMap["server_name"] = UNKNOWN;
                if(wordLines.size() == 2 && brace == 1){
                    server.serverName = wordLines[1];
                }
                else
                    throw runtime_error("Error line " + toString(lineNum) + ": bad config server_name:" + *(--wordLines.end()));
                break;

            //(port)
            case PORT:
                configTypeMap["port"] = UNKNOWN;
                if(wordLines.size() == 2){
                    port = parsePort(wordLines[1]);
                }
                else
                    throw runtime_error("Error line " + toString(lineNum) + ": bad config port:" + *(--wordLines.end()));
                break;

            //(location)    
            case LOCATION:
                if(wordLines.size() == 3 && wordLines[2] == "{" && brace == 1){
                    setValuesLocation(configTypeMap);
                    location = Location();
                    if(wordLines[1][0] != '/')
                        throw runtime_error("Error line " + toString(lineNum) + ": not valid location dir:" + wordLines[1]);
                    if(checkRoutesServer(server.getRoutes(), wordLines[1]) == 1)
                        throw runtime_error("Error line " + toString(lineNum) + ": duplicated location dir:" + wordLines[1]);
                    routeKey = wordLines[1];
                    ++brace;
                }
                else
                    throw runtime_error("Error line " + toString(lineNum) + ": bad config location:" + *(--wordLines.end()));
                break;

            //(redirect)    
            case REDIRECT:
                configTypeMap["redirect"] = UNKNOWN;
                if(wordLines.size() == 2 && brace == 2){
                    if(wordLines[1][0] != '/')
                        throw runtime_error("Error line " + toString(lineNum) + ": not valid redirect dir:" + wordLines[1]);

                    location.redirectionUrl = wordLines[1];
                }
                else
                    throw runtime_error("Error line " + toString(lineNum) + ": bad config redirection:" + *(--wordLines.end()));
                break;
            
            //(error_page)
            case ERROR_PAGE:
                if(wordLines.size() == 3 && brace == 1){
                    if(!isClientErrorHttpCode(wordLines[1]))
                        throw runtime_error("Error line " + toString(lineNum) + ": error_page status code error: " + wordLines[1]);

                    if(server.getPageStatus(wordLines[1]) != "")
                        throw runtime_error("Error line " + toString(lineNum) + ": error_page status code already used: " + wordLines[1]);

                    server.addErrorPage(wordLines[1], wordLines[2]);
                }
                else
                    throw runtime_error("Error line " + toString(lineNum) + ": bad config error_page:" + *(--wordLines.end()));
                break;
            
            //(root)
            case ROOT:
                configTypeMap["root"] = UNKNOWN;
                if(wordLines.size() == 2 && brace == 2){
                    if(wordLines[1][0] != '/')
                        throw runtime_error("Error line " + toString(lineNum) + ": not valid root dir:" + wordLines[1]);

                    location.root = wordLines[1];
                }
                else
                    throw runtime_error("Error line " + toString(lineNum) + ": bad config root:" + *(--wordLines.end()));
                break;

            //(index)
            case INDEX:
                configTypeMap["index"] = UNKNOWN;
                if(wordLines.size() == 2 && brace == 2){
                    location.defaultPath = wordLines[1];
                }
                else
                    throw runtime_error("Error line " + toString(lineNum) + ": bad config index:" + *(--wordLines.end()));
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
                        throw runtime_error("Error line " + toString(lineNum) + ": auto_index not valid config " + wordLines[1]);
                }
                else
                    throw runtime_error("Error line " + toString(lineNum) + ": bad config auto_index:" + *(--wordLines.end()));
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
                    if(aux[DELETE] == 0)
                        location.methods[DELETE] = 0;
                }
                else
                    throw runtime_error("Error line " + toString(lineNum) + ": bad config methods:" + *(--wordLines.end()));
                break;

            case CGI:
                if (wordLines.size() == 3 && brace == 2) {
                    if(wordLines[1].find('.') != 0 || wordLines[1].find('.', 1) != std::string::npos)
                        throw runtime_error("Error line " + toString(lineNum) + ": CGI error, not valid extension: " + wordLines[1]);
                    if(wordLines[2][0] != '/')
                        throw runtime_error("Error line " + toString(lineNum) + ": CGI error, invalid executable route: " + wordLines[2]);
                    if(location.cgi[wordLines[1]] == ""){
                        location.cgi[wordLines[1]] = wordLines[2];
                    }
                    else{
                        throw runtime_error("Error line " + toString(lineNum) + ": duplicated CGI rule:" + *(--wordLines.end()));
                    }

                } else {
                    throw runtime_error("Error line " + toString(lineNum) + ": bad config CGI:" + *(--wordLines.end()));
                }
                break;

            //(})   
            case BRACE_CLOSE:
                if (wordLines[0] == "}" && wordLines.size() == 1){
                    --brace;
                    if(brace == 0){
                        if(configTypeMap["port"] != UNKNOWN){
                            throw runtime_error("Error line " + toString(lineNum) + ": bad config, missing port");
                        }
                        if(!server.getNumRoutes()){
                            throw runtime_error("Error line " + toString(lineNum) + ": bad config, missing location");
                        }

                        if(checkAllRoutesByServerVec(serverDefinitions[port], server.getKeysRoutes()) == 1){
                            throw runtime_error("Error line " + toString(lineNum) + ": one route or more is already registered to this port");
                        }
                        
                        serverDefinitions[port].insert(serverDefinitions[port].begin(), server);
                        setValuesConfigTypeMap(configTypeMap, 1);
                    }
                    if(brace == 1){
                        if(configTypeMap["index"] != UNKNOWN && configTypeMap["root"] != UNKNOWN && configTypeMap["redirection"] != UNKNOWN)
                            throw runtime_error("Error line " + toString(lineNum) + ": invalid location");

                        server.addLocation(routeKey, location);
                        
                        setValuesConfigTypeMap(configTypeMap);
                    }
                }
                else
                    throw runtime_error("Error line " + toString(lineNum) + ": bad config: " + wordLines[0]);
                break;
            default:
                throw runtime_error("Error line " + toString(lineNum) + ": bad config: " + wordLines[0]);
            break;
        }
        if(brace == 0 && configType != BRACE_CLOSE){
            throw runtime_error("Error line " + toString(lineNum) + ": bad config: " + wordLines[0]);
        }            
    }
    if(brace != 0){
        throw runtime_error("Error line " + toString(lineNum) + ": bad config: " + wordLines[0]);
    }
}
