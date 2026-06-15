#pragma once
#include <string>
#include <vector>
#include <variant>
#include "../globalvariables.hpp"


class RespParser {
public:
    static RespResult StringDecode(std::string& buffer, int& i);
    static RespResult IntegerDecode(std::string& buffer, int& i);
    static RespResult RequestDecode(std::string& buffer, int& i);
};