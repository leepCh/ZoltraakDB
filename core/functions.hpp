#include "../globalvariables.hpp"
#include "string"


class functions{
    public:
     static std::string ping(std::string arg="");
     static std::string set(const ZoltraakKey& key,const ZoltraakValue& value,int ttl);
     static ZoltraakValue get(const ZoltraakKey& key);
     static int ttl(const ZoltraakKey& key);
     static int delete_key(const ZoltraakKey& key);
     static int expire(const ZoltraakKey& key,int exp_time);
     static int changeVal(const ZoltraakKey& key,int value);
     static int snapshot(const std::string& dir);

};
