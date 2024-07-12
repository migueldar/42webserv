#include "webserv.hpp"

static void parseField(std::string fieldLine, std::map<std::string, std::string>& headers) {
	std::string fieldName, fieldValue;
	std::string::const_iterator it = fieldLine.begin();

	while (it != fieldLine.end() && *it != ':') {
		fieldName += *it;
		it++;
	}
	if (*it != ':' || !isToken(fieldName))
		throw std::runtime_error("");
	it++;

	while (it != fieldLine.end() && (*it == ' ' || *it == '\t'))
		it++;
	std::string::const_iterator rit = fieldLine.end();
	rit--;
	while (rit != it - 1 && (*rit == ' ' || *rit == '\t'))
		rit--;
	while (it <= rit) {
		fieldValue += *it;
		it++;
	}
	if (!isFieldLine(fieldValue))
		throw std::runtime_error("");

	if (headers.count(fieldName) == 1)
		headers[fieldName] = headers[fieldName].append(", " + fieldValue);
	else
		headers[fieldName] = fieldValue;
}

static std::map<std::string, std::string> parseFields(std::string fields) {
	size_t found;
	size_t prev_found = 0;
	std::map<std::string, std::string> ret;

	found = fields.find("\r\n", prev_found);
	while (found != std::string::npos) {
		parseField(fields.substr(prev_found, found - prev_found), ret);
		prev_found = found + 2;
		found = fields.find("\r\n", prev_found);
	}
	return ret;
}

static std::string removeQuotes(std::string& str) {
	std::string ret;
	for (size_t i = 0; i < str.length(); i++)
		if (str[i] != '"' && str[i] != '\'')
			ret += str[i];
	return ret;
}

static std::string getFile(std::map<std::string, std::string>& fields) {
	if (fields.count("Content-Disposition") == 0)
		throw std::runtime_error("");
	std::string field = fields.at("Content-Disposition");
	if (field.find("filename=") == std::string::npos)
		throw std::runtime_error("");

	std::string ret = field.substr(field.find("filename=") + 9, field.find(";"));
	return parsePctEncoding(removeQuotes(ret));
}

stringWrap parseMultipart(std::string& fileName, stringWrap& body) {
	size_t found = body.find("\r\n");
	stringWrap left;
	stringWrap ret;

	if (found == std::string::npos)
		throw std::runtime_error("");

	left = body.subdeque(found + 2);
	found = left.find("\r\n\r\n");

	if (found == std::string::npos)
		throw std::runtime_error("");

	std::map<std::string, std::string> fields = parseFields(left.substr(0, found + 2));
	left = left.subdeque(found + 4);

	found = left.find("\r\n");

	if (found == std::string::npos)
		throw std::runtime_error("");

	ret = left.subdeque(0, found);
	left = left.subdeque(found + 2);

	found = left.find("\r\n");

	if (found == std::string::npos)
		throw std::runtime_error("");

	fileName = getFile(fields);
	if (fileName.empty())
		throw std::runtime_error("");
	return ret;
}
