#include "./functions.hpp"
#include <unordered_map>
#include "../globalvariables.hpp"

std::unordered_map<ZoltraakKey,ZoltraakObject> db;


std::string functions::ping(std::string args){

    if(args==""){
        return "PONG";
    }

    return args;
}

std::string functions::set(const ZoltraakKey& key,const ZoltraakValue& value,int ttl){

    ZoltraakObject obj;
    
    obj.value = value;

    if (ttl > 0) {
        
        obj.expires_at = std::chrono::steady_clock::now() + std::chrono::seconds(ttl);
        obj.has_ttl = true;
    } else {
        obj.has_ttl = false;
    }

    db[key] = obj; // we are overwriting if it exists.

    return "OK";

}

ZoltraakValue functions::get(const ZoltraakKey& key,int &type){
    auto it = db.find(key);

    if(it==db.end()){
        return "";
    }

    if (it->second.has_ttl) {
        auto now = std::chrono::steady_clock::now();
        if (now >= it->second.expires_at) {
            db.erase(it);
            return ""; 
        }
    }

    ZoltraakValue val=it->second.value;

    if(std::holds_alternative<int>(val)){
        type=1;
    }
    else if(std::holds_alternative<std::string>(val)){
        type=2;
    }
    else if(std::holds_alternative<std::vector<std::string>>(val)){
        type=3;
    }

    return val;
}

int functions::expire(const ZoltraakKey& key,int exp_time){
    if(db.find(key)==db.end()){
        return 0;
    }

    db[key].expires_at=std::chrono::steady_clock::now() + std::chrono::seconds(exp_time);
    db[key].has_ttl=true;

    return 1;

}

int functions::delete_key(const ZoltraakKey& key){
    if(db.find(key)==db.end()){
        return 0;
    }

    db.erase(key);
    return 1;

}

int functions::ttl(const ZoltraakKey& key){
    auto it = db.find(key);

    if(it==db.end()){
        return -2;
    }

    if(!it->second.has_ttl){
        return -1;
    }

    auto now = std::chrono::steady_clock::now();
    if(now >= it->second.expires_at){
        db.erase(it);
        return -2;
    }

    auto remaining = std::chrono::duration_cast<std::chrono::seconds>(it->second.expires_at - now);
    return remaining.count();
}






