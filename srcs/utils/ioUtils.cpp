#include "webserv.hpp"

stringWrap readFile(int fd) {
    stringWrap	ret;
	int			bytesRead;
    char		read_buff[SIZE_READ + 1];

	memset(read_buff, 0, SIZE_READ + 1);
	do {
		bytesRead = read(fd, read_buff, SIZE_READ);
		if (bytesRead < 0)
			throw std::runtime_error("corrupted fd");
		// std::cout << "readbuf" << read_buff << std::endl;
		ret += read_buff;
	} while (bytesRead == SIZE_READ);

	return ret;
}


long writeFile(int fd, stringWrap str){
    std::string aux;
    long ret = 0;

    do
    {
    	aux = str.popFirst();
    	long written = write(fd, aux.c_str(), aux.size());
        if (written < 0) {
            return -1;
        }
        else
            ret += written;
    } while (!aux.empty());

    return ret;
}