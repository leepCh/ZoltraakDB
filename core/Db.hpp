#pragma once
#include <unordered_map>
#include <string>
#include "../globalvariables.hpp"
#include <random>
#include <algorithm>
#include <chrono>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <format>
#include <iomanip>
#include <filesystem>
#include <fstream>

class ZoltraakDB {
private:
    std::unordered_map<ZoltraakKey, ZoltraakObject> db;
    std::vector<std::string> keys_with_ttl;

    void trackTtlKey(const ZoltraakKey& key) {
        if (std::find(keys_with_ttl.begin(), keys_with_ttl.end(), key) == keys_with_ttl.end()) {
            keys_with_ttl.push_back(key);
        }
    }

    // Private constructor prevents anyone else from making copies of this database
    ZoltraakDB() {}

    
    

public:
    
    static ZoltraakDB& getInstance() {
        static ZoltraakDB instance;
        return instance;
    }

    // The above thing is called meyer's singleton. 
    //It ensures that only one instance is created (helpful for our database security in runtime).

    ZoltraakDB(const ZoltraakDB&) = delete;
    void operator=(const ZoltraakDB&) = delete;
    
    void put(const ZoltraakKey& key, const ZoltraakObject& obj) {
        db[key] = obj;
        if(obj.has_ttl==true){
            trackTtlKey(key);
        }
    }

    ZoltraakObject* fetch(const ZoltraakKey& key) {
        auto it = db.find(key);
        if (it == db.end()) return nullptr;
        return &(it->second);
    }
    
    bool remove(const ZoltraakKey& key) {
        return db.erase(key) > 0;
    }

    bool expire(const ZoltraakKey& key, int exp_time) {
        auto it = db.find(key);
        if (it == db.end()) return false;

        it->second.expires_at = std::chrono::steady_clock::now() + std::chrono::seconds(exp_time);
        it->second.has_ttl = true;
        trackTtlKey(key);
        return true;
    }

    size_t size() const { 
        return db.size(); 
    }

    bool activeExpireCycle() {
        if (keys_with_ttl.empty()) return false;

        size_t total_ttl_keys = keys_with_ttl.size();
        size_t sample_size = std::min(size_t(20), total_ttl_keys);
        size_t checked_count = 0;
        size_t expired_count = 0;
        auto now = std::chrono::steady_clock::now();

        // static std::random_device rd;
        // static std::mt19937 gen(rd());

        for (int i = 0; i < sample_size && !keys_with_ttl.empty(); ++i) {
            checked_count++;
             printf("about to run active expiry\n");
            // std::uniform_int_distribution<size_t> dis(0, keys_with_ttl.size() - 1);
            size_t random_idx = rand() % keys_with_ttl.size();
            std::string target_key = keys_with_ttl[random_idx];

            auto it = db.find(target_key);
            if (it != db.end() && it->second.has_ttl) {
                if (now >= it->second.expires_at) {
                    db.erase(it);
                    expired_count++;

                    keys_with_ttl[random_idx]=keys_with_ttl.back();
                    keys_with_ttl.pop_back();
                }
            } else {
                keys_with_ttl[random_idx]=keys_with_ttl.back();
                keys_with_ttl.pop_back();
            }

           
        }

        return expired_count > 0 && expired_count >= std::max(size_t(1), checked_count / 4);
    }

    int exportToCSV(const std::string& directory_path) {

    pid_t pid = fork();

    if(pid<0){
        return -1;
    }

    if(pid==0){
   
    std::filesystem::create_directories(directory_path);

    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    
    ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");
    

    std::string full_path = directory_path + "/snapshot_" + ss.str() + ".csv";

   
    std::ofstream csvFile(full_path, std::ios::trunc);
    if (!csvFile.is_open()) return -1;

   
    csvFile << "Key,Value,Has_TTL\n";

    for (const auto& [key, obj] : db) {
        if (std::holds_alternative<std::string>(obj.value)) {
            std::string actual_value = std::get<std::string>(obj.value);

            csvFile << key << "," 
                    << actual_value << "," 
                    << obj.has_ttl << "\n";
        }
    }

    csvFile.close();

    return 0;
}

  int status;
   
  waitpid(-1, &status, 0);

  if(status==0){
     return 0;
  }
  else{
    return -1;
  }



}




};
