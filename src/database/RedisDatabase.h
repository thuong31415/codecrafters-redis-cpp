#ifndef REDIS_DATABASE_H
#define REDIS_DATABASE_H
#include <string>
#include <unordered_map>

class RedisDatabase {
public:
    RedisDatabase();

    ~RedisDatabase();

    void set(const std::string &key, const std::string &value);

    std::string get(const std::string &key);

    void del(const std::string &key);

private:
    std::unordered_map<std::string, std::string> store_{};
};


#endif //REDIS_DATABASE_H
