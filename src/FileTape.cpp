#include "FileTape.hpp"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <stdexcept>

TapeConfig FileTape::global_config;

namespace {
std::streamoff GetOffset64(uint64_t position) {
  return static_cast<std::streamoff>(position * sizeof(int32_t));
}
}  // namespace

FileTape::FileTape(const std::string &filename, OpenMode mode) {
  auto flags = std::ios::in | std::ios::out | std::ios::binary;
  if (mode == OpenMode::kCreate) {
    flags |= std::ios::trunc;
  }
  file_.open(filename, flags);
  if (file_.is_open()) {
    position_ = 0;
    file_.seekg(0, std::ios::end);
    const auto bytes = file_.tellg();
    if (bytes < 0 || bytes % sizeof(int32_t) != 0) {
      throw std::runtime_error(
          "FileTape: file size is not a multiple of 4 bytes");
    }
    size_ = static_cast<uint64_t>(bytes / sizeof(int32_t));
    file_.seekg(0, std::ios::beg);
    file_.seekp(0, std::ios::beg);
    file_position_ = 0;
  } else {
    throw std::runtime_error("FileTape: file " + filename + " not open");
  }
}
void FileTape::write(int32_t value) {
  applyDelay(global_config.write_delay_ms);

  if (position_ > size_) {
    throw std::runtime_error("Write error, end of tape");
  }
  if (file_position_ != position_) {
    file_.clear();
    file_.seekg(GetOffset64(position_), std::ios::beg);
    file_.seekp(GetOffset64(position_), std::ios::beg);
    file_position_ = position_;
  }
  file_.write(reinterpret_cast<const char *>(&value), sizeof(int32_t));

  if (!file_) {
    throw std::runtime_error("Write error");
  }
  size_ = std::max(size_, position_ + 1);
  ++file_position_;
}
int32_t FileTape::read() {
  applyDelay(global_config.read_delay_ms);

  int32_t value;
  if (position_ >= size_) {
    throw std::runtime_error("Read error, end of tape");
  }
  if (file_position_ != position_) {
    file_.clear();
    file_.seekg(GetOffset64(position_), std::ios::beg);
    file_.seekp(GetOffset64(position_), std::ios::beg);
    file_position_ = position_;
  }

  file_.read(reinterpret_cast<char *>(&value), sizeof(int32_t));

  if (!file_) {
    throw std::runtime_error("Read error, bad file");
  }
  ++file_position_;
  return value;
}
void FileTape::moveForward() {
  applyDelay(global_config.move_delay_ms);
  if (position_ < size_) {
    ++position_;
  }
}
void FileTape::moveBackward() {
  applyDelay(global_config.move_delay_ms);
  file_.clear();
  if (position_ > 0) {
    --position_;
  }
}
void FileTape::rewind() {
  applyDelay(global_config.rewind_delay_ms);
  file_.clear();
  file_.seekg(0, std::ios::beg);
  file_.seekp(0, std::ios::beg);
  position_ = 0;
  file_position_ = 0;
}
bool FileTape::isEnd() const { return position_ >= size_; }
uint64_t FileTape::size() const { return size_; }

FileTape::~FileTape() {
  file_.flush();
  file_.close();
}

void FileTape::flush() { file_.flush(); }