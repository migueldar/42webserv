
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