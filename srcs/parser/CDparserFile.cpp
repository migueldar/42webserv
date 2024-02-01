#include "webserv.hpp"

ParserFile::ParserFile(std::string routeToParserFile) : configParserFile(routeToParserFile == "" ? DEFAULT_CONFIG_ParserFile : routeToParserFile ), numParserFile(0) {
    if (!configParserFile.is_open()) {
        throw std::runtime_error("Error: ParserFile couldn't be opened: " + routeToParserFile);
    }

    parseFile();
}

void ParserFile::parseFile() {
    std::string line;
    std::vector<std::string> wordLines;
    std::vector<std::string>::iterator it;
    int32_t brace = 0;

    while (std::getline(configParserFile, line)) {
        wordLines = splitString(line, ' ');
        it = wordLines.begin();

        if (wordLines.empty() || (*it)[0] == '#'){
            continue;
        }
    
        if (*it == "server" && *(++it) == "{" && ++it == wordLines.end()){
            ++brace += fillServer();
        }
        else{
            throw std::runtime_error("Error: bad config");
        }
    }

    if (brace != 0)
        throw std::runtime_error("Error: Open braket");
}

ConfigType ParserFile::stringToConfigType(const std::string& str) {
    std::map<std::string, ConfigType>::iterator it = configTypeMap.find(str);
    return (it != configTypeMap.end()) ? it->second : UNKNOWN;
}

void ParserFile::serValuesConfigTypeMap(void){
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

int32_t ParserFile::fillServer() {
    int32_t brace = 0;
    std::string line;
    std::vector<std::string> wordLines;
    std::vector<std::string>::iterator it;

    serValuesConfigTypeMap();

    while (std::getline(configParserFile, line)) {
        wordLines = splitString(line, ' ');
        it = wordLines.begin();

        if (wordLines.empty() || (*it)[0] == '#'){
            continue;
        }

        ConfigType configType = stringToConfigType(*it);

        switch (configType) {
            case SERVER_NAME:
                configTypeMap["server_name"] = UNKNOWN;
                std::cout << "SERVER NAME FOUND" << std::endl;
                break;
            case LOCATION:
                std::cout << "LOCATION FOUND" << std::endl;
                break;
            case REDIRECT:
                configTypeMap["redirect"] = UNKNOWN;
                std::cout << "REDIRECT FOUND" << std::endl;
                break;
            case ERROR_PAGE:
                std::cout << "ERROR PAGE FOUND" << std::endl;
                break;
            case PORT:
                configTypeMap["port"] = UNKNOWN;
                std::cout << "PORT FOUND" << std::endl;
                break;
            case ROOT:
                configTypeMap["root"] = UNKNOWN;
                std::cout << "ROOT FOUND" << std::endl;
                break;
            case INDEX:
                configTypeMap["index"] = UNKNOWN;
                std::cout << "INDEX FOUND" << std::endl;
                break;
            case METHODS:
                configTypeMap["methods"] = UNKNOWN;
                std::cout << "METHODS FOUND" << std::endl;
                break;
            case BRACE_CLOSE:
                if (++it == wordLines.end()){
                    return (--brace);
                }
                else
                    throw std::runtime_error("Error: bad config: " + *it);
                break;
            default:
                throw std::runtime_error("Error: bad config: " + *it);
            break;
        }
    }
    return (brace);
}





ParserFile::~ParserFile() {
    configParserFile.close();
}

void ParserFile::addPriorityId(uint32_t priorityId) {
    prioIdServ.push_back(priorityId);
}
