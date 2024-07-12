
#include "webserv.hpp"


std::vector<std::string> splitString(const std::string& input, char delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = input.find(delimiter);

    while (end < input.length()) {
        if (end > start) {  
            tokens.push_back(input.substr(start, end - start));
        }
        start = end + 1;
        end = input.find(delimiter, start);
    }

    if (start < input.length()) {
        tokens.push_back(input.substr(start));
    }

    return tokens;
}


short stringToShort(const std::string& str) {
    std::istringstream stream(str);
    short result = 0;

    if (!(stream >> result) || !stream.eof()) {
        throw std::invalid_argument("Error en la conversión de cadena a short");
    }

    return result;
}

unsigned long stringToUnsignedLong(const std::string& str) {
    std::istringstream stream(str);
    unsigned long result = 0;

    if (!(stream >> result) || !stream.eof()) {
        throw std::invalid_argument("Error en la conversión de cadena a unsigned long");
    }

    return result;
}