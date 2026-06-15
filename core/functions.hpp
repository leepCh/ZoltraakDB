#include "../globalvariables.hpp"
#include "string"


class functions{
    public:
     static std::string ping(std::string arg="");
     static std::string set(const ZoltraakKey& key,const ZoltraakValue& value,int ttl);
     static ZoltraakValue get(const ZoltraakKey& key,int &type);
     static int ttl(const ZoltraakKey& key);

};
