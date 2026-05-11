#pragma once
#include <cstdint>

class TapeInterface {
 public:
  virtual void write(int32_t value) = 0;
  virtual int32_t read() = 0;
  virtual ~TapeInterface() = default;

  virtual void moveForward() = 0;
  virtual void moveBackward() = 0;

  virtual void rewind() = 0;
  virtual bool isEnd() const = 0;
  virtual uint64_t size() const = 0;
};
