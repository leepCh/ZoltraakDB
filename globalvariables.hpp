#pragma once
#include <string>
#include <vector>
#include <variant>
#include <chrono>

#define MAX_KEYS 500000

struct RespResult;

using RespData = std::variant<int, std::string, std::vector<RespResult>>;

struct RespResult {
    char type;
    RespData data;
    bool success = false;

    // RespResult() : type('\0'), data(0), success(false) {}
    // RespResult(char t, RespData d, bool s) : type(t), data(d), success(s) {}
};

using ZoltraakKey = std::string;

using ZoltraakValue = std::variant<int, std::string, std::vector<std::string>>;

struct ZoltraakObject {
    ZoltraakValue value;
    std::chrono::time_point<std::chrono::steady_clock> expires_at;
    bool has_ttl = false;
};
