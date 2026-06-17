#include "./functions.hpp"
#include <unordered_map>
#include "../globalvariables.hpp"
#include "./Db.hpp"


std::string functions::ping(std::string args){

    if(args==""){
        return "PONG";
    }

    return args;
}

std::string functions::set(const ZoltraakKey& key,const ZoltraakValue& value,int ttl){

    ZoltraakDB& zdb = ZoltraakDB::getInstance();

    ZoltraakObject obj;
    
    obj.value = value;

    if (ttl > 0) {
        
        obj.expires_at = std::chrono::steady_clock::now() + std::chrono::seconds(ttl);
        obj.has_ttl = true;
    } else {
        obj.has_ttl = false;
    }

    zdb.put(key,obj);

    // db[key] = obj; // we are overwriting if it exists.

    return "OK";

}

ZoltraakValue functions::get(const ZoltraakKey& key,int &type){

    ZoltraakDB& zdb = ZoltraakDB::getInstance();

    auto it = zdb.fetch(key);

    if(it==nullptr){
        return "";
    }

    if (it->has_ttl) {
        auto now = std::chrono::steady_clock::now();
        if (now >= it->expires_at) {
            zdb.remove(key);
            return ""; 
        }
    }

    ZoltraakValue val=it->value;

    return val;
}

int functions::expire(const ZoltraakKey& key,int exp_time){

    ZoltraakDB& zdb = ZoltraakDB::getInstance();

    return zdb.expire(key, exp_time) ? 1 : 0;

}

int functions::delete_key(const ZoltraakKey& key){

    ZoltraakDB& zdb = ZoltraakDB::getInstance();

    if(zdb.fetch(key)==nullptr){
        return 0;
    }

    zdb.remove(key);
    return 1;

}

int functions::ttl(const ZoltraakKey& key){

    ZoltraakDB& zdb = ZoltraakDB::getInstance();

    auto it = zdb.fetch(key);

    if(it==nullptr){
        return -2;
    }

    if(!it->has_ttl){
        return -1;
    }

    auto now = std::chrono::steady_clock::now();
    if(now >= it->expires_at){
        zdb.remove(key);
        return -2;
    }

    auto remaining = std::chrono::duration_cast<std::chrono::seconds>(it->expires_at - now);
    return remaining.count();
}





