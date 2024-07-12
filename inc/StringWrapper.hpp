#ifndef STRINGWRAPPER_HPP
#define STRINGWRAPPER_HPP

#include <string>
#include <deque>
#include <vector>
#include <iostream>

#define MAX_STRING_SIZE 100000

//to store large strings in non-contiguous memory
//to work propertly everything should be in canonical mode
//that is all nodes but the last one must have size MAX_STRING_SIZE 
class stringWrap {
	private:
		std::deque<std::vector<char> > deq;
	public:
		stringWrap();
		stringWrap(const stringWrap& other);
		~stringWrap(); 
		stringWrap& operator=(const stringWrap& other);
		stringWrap& operator=(const std::string& other);
		stringWrap&	operator+=(const std::string& data);
		//cannot be a reference because if you add yourself it fails
		stringWrap&	operator+=(const stringWrap other);
		stringWrap& addData(const char* str, size_t size);
		char*		popFirst(int& len);
		bool		empty() const;
		std::string	substr(size_t begin = 0, size_t len = std::string::npos) const;
		stringWrap	subdeque(size_t begin = 0, size_t len = std::string::npos) const;
		size_t		length() const;
		//if length of string to find is more than MAX_STRING_SIZE doesnt work
		size_t		find(std::string toFind, size_t pos = 0) const;
		char&		operator[](size_t pos);
		const std::deque<std::vector<char> >& getDeque() const;
};

std::ostream &operator<<(std::ostream &o, const stringWrap& prt);

#endif