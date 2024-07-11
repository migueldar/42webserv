#include "webserv.hpp"

stringWrap readFile(int fd) {
    stringWrap	ret;
	int			bytesRead;
    char		read_buff[SIZE_READ + 1];

	do {
		memset(read_buff, 0, SIZE_READ + 1);
		bytesRead = read(fd, read_buff, SIZE_READ);
		if (bytesRead < 0) {
			close(fd);
			throw std::runtime_error("corrupted fd");
		}
		// std::cout << "readbuf" << read_buff << std::endl;
		ret += read_buff;
	} while (bytesRead == SIZE_READ);

	close(fd);
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
			close(fd);
            return -1;
        }
        else
            ret += written;
    } while (!aux.empty());

	close(fd);
    return ret;
}