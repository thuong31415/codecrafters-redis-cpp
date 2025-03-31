#ifndef REDIS_DATABASE_H
#define REDIS_DATABASE_H
#include <string>
#include <unordered_map>

class RedisDatabase {
public:
    RedisDatabase();

    ~RedisDatabase();

    void set(const std::string &key, const std::string &value);

    void set(const std::string &key, const std::string &value, int64_t ttl);

    std::string get(const std::string &key);

    void del(const std::string &key);

private:
    std::unordered_map<std::string, std::pair<std::string, int64_t> > store_{};
};


#endif //REDIS_DATABASE_H
