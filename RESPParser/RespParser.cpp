#include "RespParser.hpp"
#include <string>
#include <vector>
#include <cstdlib>
#include <stdexcept>


RespResult RespParser::StringDecode(std::string& buffer, int& i) {
    std::string answer = "";
    
    while (i < buffer.size() && buffer[i] != '\r') {
        answer += buffer[i];
        i++;
    }
    i += 2; 
    
    return {'$', answer, true};
}

RespResult RespParser::IntegerDecode(std::string& buffer, int& i) {
    std::string stringAnswer = "";
    
    while (i < buffer.size() && buffer[i] != '\r') {
        stringAnswer += buffer[i];
        i++;
    }
    i += 2; 

    try {
        int answer = std::stoi(stringAnswer);
        return {':', answer, true};
    }
    catch (...) {
        return {'-', "Error parsing integer", false};
    }
}

RespResult RespParser::RequestDecode(std::string& buffer, int& i) {
    if (i >= buffer.size()) return {'\0', "EOF", false};

    char token = buffer[i];

    if (token == '*') {
        i++;
        RespResult countResult = IntegerDecode(buffer, i);
        if (!countResult.success) return countResult;
        
        int arraySize = std::get<int>(countResult.data);
        std::vector<RespResult> arrayData;

       
        for (int count = 0; count < arraySize; count++) {
            RespResult subResult = RequestDecode(buffer, i);
            if (subResult.success) {
                arrayData.push_back(subResult);
            } else {
                return subResult; 
            }
        }

        return {'*', arrayData, true};
    }
    else if (token == '$') {
        i++;
        
        RespResult lenResult = IntegerDecode(buffer, i); 
        return StringDecode(buffer, i);
    }
    else if (token == ':') {
        i++;
        return IntegerDecode(buffer, i);
    }
    else if (token == '+') {
        i++; 
        return StringDecode(buffer, i);
    }

    return {'-', "Unknown RESP token", false};
}