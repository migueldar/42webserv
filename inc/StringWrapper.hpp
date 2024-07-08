#ifndef STRINGWRAPPER_HPP
#define STRINGWRAPPER_HPP

#include <string>
#include <deque>

#define MAX_STRING_SIZE 100000

//to store large strings in non-contiguous memory
//to work propertly everything should be in canonical mode
//that is all nodes but the last one must have size MAX_STRING_SIZE 
class stringWrap {
	private:
		std::deque<std::string> deq;
	public:
		stringWrap();
		stringWrap(const stringWrap& other);
		~stringWrap();
		stringWrap& operator=(const stringWrap& other);
		void		addData(std::string data);
		std::string	popFirst();
		bool		empty() const;
		std::string	substr(size_t begin = 0, size_t len = std::string::npos) const;
		stringWrap	subdeque(size_t begin = 0, size_t len = std::string::npos) const;
		size_t		length() const;
		//if length of string to find is more than MAX_STRING_SIZE doesnt work
		size_t		find(std::string toFind, size_t pos = 0) const;
		char&		operator[](size_t pos);
};

#endif