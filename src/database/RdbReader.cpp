#include "RdbReader.h"

#include <cstdint>
#include <iostream>
#include <chrono>

constexpr uint8_t LENGTH_MASK = 0xC0; // Mask to extract the first 2 bits
constexpr uint8_t LENGTH_6BIT = 0x00; // Length encoded in 6 bits
constexpr uint8_t LENGTH_14BIT = 0x40; // Length encoded in 14 bits
constexpr uint8_t LENGTH_32BIT = 0x80; // Length encoded in 32 bits
constexpr uint8_t SPECIAL_ENCODING = 0xC0; // Special encoding

constexpr uint8_t SPECIAL_INT8 = 0; // 8-bit integer
constexpr uint8_t SPECIAL_INT16 = 1; // 16-bit integer
constexpr uint8_t SPECIAL_INT32 = 2; // 32-bit integer
constexpr uint8_t SPECIAL_LZF = 3; // LZF compression
constexpr uint8_t SPECIAL_UNKNOWN = 4; // Unknown encoding


RdbReader::~RdbReader() {
    if (file_.is_open()) {
        file_.close();
    }
}

void RdbReader::Process(const std::string &filename) {
    file_.open(filename, std::ios::binary);
    if (!file_.is_open()) {
        std::cerr << "Could not open file " << filename << std::endl;
    }
    ProcessRdbFile();
}


std::string RdbReader::ReadString() {
    uint8_t byte{};
    if (!file_.read(reinterpret_cast<char *>(&byte), 1)) {
        std::cerr << "Failed to read length byte at position " << file_.tellg() << std::endl;
        return "";
    }

    const uint8_t length_type = byte & LENGTH_MASK;
    uint32_t length = 0;

    auto read_bytes = [&](char *buffer, const size_t size) -> bool {
        file_.read(buffer, size);
        const bool success = file_.gcount() == size;
        if (!success) {
            std::cerr << "Failed to read " << size << " bytes at position " << file_.tellg() << ", got " << file_.
                    gcount() << std::endl;
        }
        return success;
    };

    switch (length_type) {
        case LENGTH_6BIT: {
            length = byte & 0x3F;
            break;
        }
        case LENGTH_14BIT: {
            uint8_t next_byte;
            if (!read_bytes(reinterpret_cast<char *>(&next_byte), 1)) return "";
            length = (byte & 0x3F) << 8 | next_byte;
            break;
        }
        case LENGTH_32BIT: {
            uint32_t len;
            if (!read_bytes(reinterpret_cast<char *>(&len), 4)) return "";
            length = __builtin_bswap32(len);
            break;
        }
        case SPECIAL_ENCODING: {
            const uint8_t special_type = byte & 0x3F;
            return ReadSpecialEncoding(special_type);
        }
        default:
            std::cerr << "Unknown length type: " << static_cast<int>(length_type) << std::endl;
            return "UNKNOWN_LENGTH_TYPE";
    }

    std::string result(length, '\0');
    if (!read_bytes(&result[0], length)) {
        return "";
    }

    return result;
}

std::string RdbReader::ReadSpecialEncoding(const uint8_t type) {
    switch (type) {
        case SPECIAL_INT8: {
            int8_t val;
            if (!file_.read(reinterpret_cast<char *>(&val), 1)) return "";
            return std::to_string(val);
        }
        case SPECIAL_INT16: {
            ushort val;
            if (!file_.read(reinterpret_cast<char *>(&val), 2)) return "";
            val = __builtin_bswap16(val);
            return std::to_string(val);
        }
        case SPECIAL_INT32: {
            uint32_t val;
            if (!file_.read(reinterpret_cast<char *>(&val), 4)) return "";
            val = __builtin_bswap32(val);
            return std::to_string(static_cast<int32_t>(val));
        }
        case SPECIAL_LZF:
            return "LZF_NOT_SUPPORTED";
        default:
            return "UNKNOWN_ENCODING";
    }
}

void RdbReader::ProcessRdbFile() {
    char magic[9];
    if (!file_.read(magic, 9)) {
        return;
    }

    std::string magic_str(magic, 5);
    if (magic_str != "REDIS") {
        return;
    }
    std::cout << "Redis version: " << std::string(magic + 5, 4) << std::endl;

    uint8_t op_code{};
    while (file_.read(reinterpret_cast<char *>(&op_code), 1)) {
        if (op_code == 0xFF) {
            break;
        }

        if (op_code == 0xFA) {
            std::string key = ReadString();
            std::string value = ReadString();
            if (key.empty() || value.empty()) {
                std::cerr << "Error: Unable to read auxiliary field at position " << file_.tellg() << std::endl;
                return;
            }
        }

        if (op_code == 0xFE) {
            uint8_t db_number;
            if (!file_.read(reinterpret_cast<char *>(&db_number), 1)) {
                std::cerr << "Error: Unable to read database selector at position " << file_.tellg() << std::endl;
                return;
            }
        }

        if (op_code == 0xFC) {
            uint64_t ttl_ms;
            if (!file_.read(reinterpret_cast<char *>(&ttl_ms), 8)) {
                return;
            }
            uint8_t valueType;
            if (!file_.read(reinterpret_cast<char *>(&valueType), 1)) {
                return;
            }
            if (valueType == 0x00) {
                const std::string key = ReadString();
                const std::string value = ReadString();
                if ("UNKNOWN_ENCODING" != key && !key.empty() && !value.empty()) {
                    data_[key] = {value, ttl_ms};
                }
            }
        }

        if (op_code == 0xFD) {
            uint32_t ttl_seconds;
            if (!file_.read(reinterpret_cast<char *>(&ttl_seconds), 4)) {
                return;
            }
            uint8_t valueType;
            if (!file_.read(reinterpret_cast<char *>(&valueType), 1)) {
                return;
            }
            if (valueType == 0x00) {
                const std::string key = ReadString();
                const std::string value = ReadString();
                if ("UNKNOWN_ENCODING" != key && !key.empty() && !value.empty()) {
                    uint64_t ttl_ms = static_cast<uint64_t>(ttl_seconds) * 1000;
                    data_[key] = {value, ttl_ms};
                }
            }
        }

        if (op_code == 0xFB) {
            uint8_t size1, size2;
            if (!file_.read(reinterpret_cast<char *>(&size1), 1) || !file_.read(reinterpret_cast<char *>(&size2), 1)) {
                std::cerr << "Error reading size after op_code 0xFB at position " << file_.tellg() << std::endl;
                return;
            }
            continue;
        }

        if (op_code == 0x00) {
            const std::string key = ReadString();
            const std::string value = ReadString();

            if ("UNKNOWN_ENCODING" != key && !key.empty() && !value.empty()) {
                data_[key] = {value, -1};
            }
        }
    }
}

std::vector<std::string> RdbReader::GetKeys() const {
    std::vector<std::string> keys;
    for (const auto &key: data_ | std::views::keys) {
        keys.push_back(key);
    }
    return keys;
}
