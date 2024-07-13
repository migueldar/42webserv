#include "webserv.hpp"
#include <algorithm>

static char* vectorToPointer(const std::vector<char>& vec) {
	char* ret = new char[vec.size()];
	memcpy(ret, vec.data(), vec.size());
	return ret;
}

static char* vectorToPointerNull(const std::vector<char>& vec) {
	char* ret = new char[vec.size() + 1];
	memcpy(ret, vec.data(), vec.size());
	ret[vec.size()] = 0;
	return ret;
}

static std::vector<char> subVec(const std::vector<char>& vec, size_t begin = 0, size_t size = std::string::npos) {
	if (size >= vec.size() - begin)
		return std::vector<char>(vec.begin() + begin, vec.end());
	return std::vector<char>(vec.begin() + begin, vec.begin() + begin + size);
}

stringWrap::stringWrap() {}

stringWrap::stringWrap(const stringWrap &other) {
	*this = other;
}

stringWrap::~stringWrap() {}

stringWrap& stringWrap::operator=(const std::string& other) {
	this->deq.clear();
	this->addData(other.c_str(), other.size());
	return *this;
}

stringWrap& stringWrap::operator=(const stringWrap &other) {
	deq = other.deq;
	return *this;
}

stringWrap& stringWrap::operator+=(const std::string& other) {
	this->addData(other.c_str(), other.size());
	return *this;
}

stringWrap& stringWrap::operator+=(const stringWrap other) {
	char* aux;
	for (std::deque<std::vector<char> >::const_iterator it = other.deq.begin(); it != other.deq.end(); it++) {
		aux = vectorToPointer(*it);
		this->addData(aux, it->size());
		delete[] aux;
	}
	return *this;
}

stringWrap& stringWrap::addData(const char* str, size_t size) {
	size_t added = 0;
	if (deq.empty() && size != 0)
		deq.push_back(std::vector<char>());
	while (added < size) {
		if (deq.back().size() == MAX_STRING_SIZE)
			deq.push_back(std::vector<char>());
		std::vector<char>& last = deq.back();
		size_t sizeAppend = MAX_STRING_SIZE - last.size();
		if (sizeAppend + added >= size)
			last.insert(last.end(), str + added, str + size);
		else
			last.insert(last.end(), str + added, str + added + sizeAppend);
		added += sizeAppend;
	}
	return *this;
}

char* stringWrap::popFirst(int& len) {
	if (deq.empty()) {
		len = 0;
		return NULL;
	}

	const std::vector<char>& front = deq.front();
	char* ret = vectorToPointer(front);
	len = front.size();
	deq.pop_front();

	return ret;
}

bool stringWrap::empty() const {
	return deq.empty();
}

size_t stringWrap::length() const {
	if (deq.size() == 0)
		return 0;
	return MAX_STRING_SIZE * (deq.size() - 1) + deq.back().size();
}

//its fine that it returns string in all the occurences, there will be no zeros
std::string stringWrap::substr(size_t begin, size_t len) const {
	std::string			ret;
	std::vector<char>	aux;
	char*				auxStr;

	for (size_t i = begin; i - begin < len;) {
		if (i >= length())
			break;
		aux = subVec(deq[i / MAX_STRING_SIZE], i % MAX_STRING_SIZE, begin + len - i);
		auxStr = vectorToPointerNull(aux);
		ret += auxStr;
		delete[] auxStr;
		if (i == begin)
			i -= i % MAX_STRING_SIZE;
		i += len >= MAX_STRING_SIZE + i - begin ? MAX_STRING_SIZE : begin + len - i;
	}
	return ret;
}

stringWrap stringWrap::subdeque(size_t begin, size_t len) const {
	stringWrap			ret;
	std::vector<char>	aux;
	char*				auxStr;

	for (size_t i = begin; i - begin < len;) {
		if (i >= length())
			break;
		aux = subVec(deq[i / MAX_STRING_SIZE], i % MAX_STRING_SIZE, begin + len - i);
		auxStr = vectorToPointer(aux);
		ret.addData(auxStr, aux.size());
		delete[] auxStr;
		if (i == begin)
			i -= i % MAX_STRING_SIZE;
		i += len >= MAX_STRING_SIZE + i - begin ? MAX_STRING_SIZE : begin + len - i;
	}
	return ret;
}

size_t stringWrap::find(std::string toFind, size_t pos) const {
	std::vector<char>			aux;
	size_t						i;
	std::vector<char>::iterator	it;

	i = pos / MAX_STRING_SIZE;
	if (deq.size() == 0 || i > deq.size() - 1)
		return std::string::npos;
	aux = subVec(deq[i], pos % MAX_STRING_SIZE);
	if (i + 1 < deq.size())
		aux.insert(aux.end(), deq[i + 1].begin(), deq[i + 1].end());
	it = std::search(aux.begin(), aux.end(), toFind.begin(), toFind.end());
	if (it != aux.end())
		return it - aux.begin() + pos;
	i++;

	while (i + 1 < deq.size()) {
		aux = deq[i];
		aux.insert(aux.end(), deq[i + 1].begin(), deq[i + 1].end());
		it = std::search(aux.begin(), aux.end(), toFind.begin(), toFind.end());
		if (it != aux.end())
			return it - aux.begin() + i * MAX_STRING_SIZE;
		i++;
	}
	return std::string::npos;
}

char& stringWrap::operator[](size_t pos) {
	return deq[pos / MAX_STRING_SIZE][pos % MAX_STRING_SIZE];
}

const std::deque<std::vector<char> >& stringWrap::getDeque() const {
	return deq;
}

std::ostream &operator<<(std::ostream &o, const stringWrap &prt) {
	const std::deque<std::vector<char> >& pr = prt.getDeque();

	for (std::deque<std::vector<char> >::const_iterator it = pr.begin(); it < pr.end(); it++) {
		for (std::vector<char>::const_iterator itt = it->begin(); itt < it->end(); itt++)
			o << *itt;
		o << std::endl;
	}
	return o;
}
