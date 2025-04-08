#ifndef RDB_READER_H
#define RDB_READER_H

#include <cstdint>
#include <fstream>
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>


class RdbReader {
public:
    explicit RdbReader() = default;

    ~RdbReader();

    void Process(const std::string &filename);

    std::unordered_map<std::string, std::pair<std::string, int64_t>> GetData() { return data_; };

    std::vector<std::string> GetKeys() const;

private:
    std::ifstream file_;

    std::string ReadString();

    void ProcessRdbFile();

    std::string ReadSpecialEncoding(uint8_t type);

    std::unordered_map<std::string, std::pair<std::string, int64_t>> data_{};
};


#endif //RDB_READER_H
