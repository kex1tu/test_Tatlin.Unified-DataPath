#pragma once
#include <cstdint>
#include <fstream>
#include <string>
#include <thread>

#include "TapeConfig.hpp"
#include "TapeInterface.hpp"
enum class OpenMode : uint8_t { kOpen, kCreate };
class FileTape : public TapeInterface {
 public:
  static TapeConfig global_config;
  static void setGlobalConfig(const TapeConfig& config) {
    global_config = config;
  }

  explicit FileTape(const std::string& filename,
                    OpenMode mode = OpenMode::kOpen);

  virtual void write(int32_t value) override;
  virtual int32_t read() override;
  virtual ~FileTape() override;
  virtual void moveForward() override;
  virtual void moveBackward() override;
  virtual void rewind() override;
  virtual bool isEnd() const override;
  virtual uint64_t size() const override;
  void flush();

 private:
  static void applyDelay(uint64_t delay_ms) {
    if (delay_ms > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }
  }
  std::fstream file_;
  uint64_t position_ = 0;
  uint64_t size_ = 0;
};
