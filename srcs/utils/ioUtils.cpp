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
    char* aux;
    long ret = 0;
	int toWriteLen;

	while (!str.empty()) {
    	aux = str.popFirst(toWriteLen);
    	long written = write(fd, aux, toWriteLen);
        if (written < 0) {
			close(fd);
            return -1;
        }
        else
            ret += written;
	}
	close(fd);
    return ret;
}