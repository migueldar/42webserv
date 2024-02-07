#include <iostream>
#include <map>
#include <vector>
#include <stdexcept>

enum ConfigType {
    SERVER_NAME,
    LOCATION,
    REDIRECT,
    ERROR_PAGE,
    PORT,
    ROOT,
    INDEX,
    METHODS,
    BRACE_CLOSE,
    UNKNOWN
};

template <typename Payload >
class ConfigMapper {
public:

    typedef void (*FunctionPointer)(Payload&);

    ConfigMapper(Payload& payload, const std::map<std::string, unsigned long>& configTypeMap, FunctionPointer functionArray[8])
        : payload(payload), configTypeMap(configTypeMap), functionArray(functionArray){
    }


    void processConfig(std::vector<std::string> configParserFile) {
        bool flag = 0;
        unsigned int port;
        std::string line;
        std::vector<std::string> wordLines;
        std::vector<std::string>::iterator it;

        while (std::getline(configParserFile, line)) {
            wordLines = splitString(line, ' ');
            it = wordLines.begin();

            if (wordLines.empty()){
                continue;
            }
            flag = functionArray[configTypeMap[wordLines[0]]](payload);
            if (flag == 1)
                throw std::runtime_error(*it);
        }
    }

private:

    Payload& payload;
    std::map<std::string, unsigned long> configTypeMap;
    FunctionPointer functionArray[8];
};



