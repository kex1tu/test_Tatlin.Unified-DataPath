#pragma once
#include <cctype>
#include <cstdint>
#include <fstream>
#include <string>
#include <unordered_map>

struct TapeConfig {
  uint64_t memory_limit_bytes = 67108864;  // 64 MB
  uint64_t read_delay_ms = 0;
  uint64_t write_delay_ms = 0;
  uint64_t move_delay_ms = 0;
  uint64_t rewind_delay_ms = 0;

  static std::string trim(const std::string& str) {
    auto start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
      return "";
    }
    auto end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
  }

  static TapeConfig load(const std::string& path) {
    TapeConfig config;
    std::ifstream file(path);
    if (!file.is_open()) {
      return config;
    }
    std::unordered_map<std::string, uint64_t*> fields = {
        {"memory_limit_bytes", &config.memory_limit_bytes},
        {"read_delay_ms", &config.read_delay_ms},
        {"write_delay_ms", &config.write_delay_ms},
        {"move_delay_ms", &config.move_delay_ms},
        {"rewind_delay_ms", &config.rewind_delay_ms}};

    std::string line;
    while (std::getline(file, line)) {
      line = trim(line);
      if (line.empty() || line[0] == '#') {
        continue;
      }

      auto pos = line.find('=');
      if (pos == std::string::npos) {
        continue;
      }

      std::string key = trim(line.substr(0, pos));
      std::string value_str = trim(line.substr(pos + 1));

      auto it = fields.find(key);
      if (it != fields.end()) {
        try {
          if (!value_str.empty() && value_str[0] != '-') {
            *it->second = std::stoull(value_str);
          }
        } catch (...) {
          continue;
        }
      }
    }
    return config;
  }
};