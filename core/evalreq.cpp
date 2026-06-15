#include "./functions.hpp"
#include "../globalvariables.hpp"
#include "./evalreq.hpp"
#include <algorithm>
#include <cctype>

// Encoder implementations
std::string Encoder::bulkStringEncode(const std::string &str)
{
    return "$" + std::to_string(str.length()) + "\r\n" + str + "\r\n";
}

std::string Encoder::simpleStringEncode(const std::string &str)
{
    return "+" + str + "\r\n";
}

std::string Encoder::integerEncode(int num)
{
    return ":" + std::to_string(num) + "\r\n";
}

std::string Encoder::arrayencode(const std::vector<std::string> &arr)
{
    std::string encoded = "*" + std::to_string(arr.size()) + "\r\n";

    for (const std::string &element : arr)
    {
        encoded += Encoder::bulkStringEncode(element);
    }

    return encoded;
}

// RequestEvaluator implementations
std::string RequestEvaluator::evalArray(const RespData &arr)
{
    auto &arrayData = std::get<std::vector<RespResult>>(arr);
    try
    {
        std::string cmd = std::get<std::string>(arrayData.front().data);
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

        if (cmd == "ping")
        {
            if (arrayData.size() == 2)
            {
                std::string arg = std::get<std::string>(arrayData.at(1).data);
                std::string decodedAns = functions::ping(arg);
                return Encoder::bulkStringEncode(decodedAns);
            }
            else if (arrayData.size() == 1)
            {
                return Encoder::simpleStringEncode("PONG");
            }
            else
            {
                return std::string("-Error:Incorrect number of arguments for ping\r\n");
            }
        }
        else if (cmd == "set")
        {
            if (arrayData.size() < 3 || arrayData.size() > 5)
            { // SET needs at least: cmd, key, value
                return std::string("-Error:Incorrect number of arguments for set\r\n");
            }

            // extract key
            ZoltraakKey key;
            if (std::holds_alternative<std::string>(arrayData.at(1).data))
            {
                key = std::get<std::string>(arrayData.at(1).data);
            }
            else
            {
                return std::string("-Error:Not a valid key type\r\n");
            }

            // extract value
            ZoltraakValue value;
            auto &valueToken = arrayData.at(2).data;

            if (std::holds_alternative<std::string>(valueToken))
            {
                value = std::get<std::string>(valueToken);
            }
            else if (std::holds_alternative<int>(valueToken))
            {
                value = std::get<int>(valueToken);
            }
            else
            {
                return std::string("-Error:Invalid value type\r\n");
            }

            int ttl_seconds = -1;
            bool has_ttl = false;

            // extract timestamp
            if (arrayData.size() == 5)
            {
                std::string option = "";
                if (std::holds_alternative<std::string>(arrayData.at(3).data))
                {
                    option = std::get<std::string>(arrayData.at(3).data);
                }

                std::transform(option.begin(), option.end(), option.begin(), ::tolower);

                if (option == "ex")
                {
                    if (std::holds_alternative<int>(arrayData.at(4).data))
                    {
                        ttl_seconds = std::get<int>(arrayData.at(4).data);
                        has_ttl = true;
                    }
                    else if (std::holds_alternative<std::string>(arrayData.at(4).data))
                    {
                        try
                        {
                            ttl_seconds = std::stoi(std::get<std::string>(arrayData.at(4).data));
                            has_ttl = true;
                        }
                        catch (...)
                        {
                            return std::string("-Error:value is not an integer or out of range\r\n");
                        }
                    }
                }
                else
                {
                    return std::string("-Error:Syntax error on optional parameters\r\n");
                }
            }
            return Encoder::simpleStringEncode(functions::set(key, value, ttl_seconds));
        }
        
        else if (cmd == "get")
        {
            // get takes strictly 2 args get key
            if (arrayData.size() != 2)
            {
                return std::string("-Error: Incorrect arguements for get");
            }
            // 1 arg is key

            if (!std::holds_alternative<std::string>(arrayData.at(1).data))
            {
                return std::string("-Error:Not a valid key type\r\n");
            }

            ZoltraakKey key = std::get<std::string>(arrayData.at(1).data);
            int type = 0;

            ZoltraakValue value = functions::get(key, type);

            if (std::holds_alternative<std::string>(value))
            {
                std::string value_str = std::get<std::string>(value);
                if (value_str == "")
                {
                    return "$-1\r\n";
                }
                return Encoder::bulkStringEncode(value_str);
            }
            // if it is integer
            else if (std::holds_alternative<int>(value))
            {
                int value_int = std::get<int>(value);

                return Encoder::integerEncode(value_int);
            }
            // vector
            else if (std::holds_alternative<std::vector<std::string>>(value))
            {
                std::vector<std::string> value_arr = std::get<std::vector<std::string>>(value);

                return Encoder::arrayencode(value_arr);
            }

            return std::string("-Error:Invalid value type\r\n");
        }
        else if(cmd=="ttl"){
            // ttl takes only 2 args
            if(arrayData.size()!=2){
                return std::string("-Error: Incorrect arguements for TTL");
            }

            if (!std::holds_alternative<std::string>(arrayData.at(1).data))
            {
                return std::string("-Error:Not a valid key type\r\n");
            }

            ZoltraakKey key=std::get<std::string>(arrayData.at(1).data);
            int remaining_ttl = functions::ttl(key);
            return Encoder::integerEncode(remaining_ttl);
        }
        else if(cmd=="delete"){
            if(arrayData.size()<2){
                return std::string("-Error:Incorrect no of arguements for delete");
            }
            int noofdeleted=0;

            for(int i=1;i<arrayData.size();i++){
                if(!std::holds_alternative<std::string>(arrayData.at(i).data)){

                    return std::string("-Error:Not a valid key type ")+std::to_string(noofdeleted)+std::string("keys deleted\r\n");
                }

                std::string key = std::get<std::string>(arrayData.at(i).data);

                noofdeleted+=functions::delete_key(key);

            }

            return Encoder::integerEncode(noofdeleted);
        }
        else if(cmd=="expire"){
            if(arrayData.size()!=3){
                return std::string("-Error:Incorrect no of arguements for expire");
            }

             if(!std::holds_alternative<std::string>(arrayData.at(1).data)){

                    return std::string("-Error:Not a valid key type\r\n");
                }

                std::string key = std::get<std::string>(arrayData.at(1).data);


                try
                        {
                            int expireTime = std::stoi(std::get<std::string>(arrayData.at(2).data));
                            int a = functions::expire(key,expireTime);
                            return Encoder::integerEncode(a);
                        }
                        catch (...)
                        {
                            return std::string("-Error:value is not an integer or out of range\r\n");
                        }


        }
        else
        {
            return std::string("-Error:Not a valid function\r\n");
        }
        
    }
    catch (...)
    {
        return std::string("-Error:No valid functionality exists\r\n");
    }
}

std::string RequestEvaluator::evalSimpleString(const RespData &str)
{
    std::string simpleStr = std::get<std::string>(str);
    std::transform(simpleStr.begin(), simpleStr.end(), simpleStr.begin(), ::tolower);

    if (simpleStr == "ping")
    {
        return Encoder::simpleStringEncode("PONG");
    }
    return std::string();
}

std::string RequestEvaluator::processAndEncode(const RespResult &req)
{
    if (req.type == '*')
    {
        return RequestEvaluator::evalArray(req.data);
    }
    else if (req.type == '+')
    {
        return RequestEvaluator::evalSimpleString(req.data);
    }
    return std::string();
}
