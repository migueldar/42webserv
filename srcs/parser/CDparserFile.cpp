

#include "../inc/parser.hpp"

parserFile::parserFile() : numfile(0) {
    
}

void parserFile::addPriorityId(uint32_t priorityId) {
    prioIdServ.push_back(priorityId);
}