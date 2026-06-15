#ifndef EVALREQ_HPP
#define EVALREQ_HPP

#include <string>
#include "../globalvariables.hpp"

class Encoder {
public:
	static std::string bulkStringEncode(const std::string& str);
	static std::string simpleStringEncode(const std::string& str);
	static std::string integerEncode(int num);
	static std::string arrayencode(const std::vector<std::string>& arr);
};

class RequestEvaluator {
public:
	static std::string evalArray(const RespData& arr);
	static std::string evalSimpleString(const RespData& str);
	static std::string processAndEncode(const RespResult& req);
};

#endif


