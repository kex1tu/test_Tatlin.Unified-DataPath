#include "../src/FileTape.hpp"

#include <gtest/gtest.h>

#include <fstream>
#include <limits>
#include <vector>
namespace fs = std::filesystem;

class FileTapeTest : public ::testing::Test {};

TEST_F(FileTapeTest, IO_Single) {
  std::ofstream file("test_tape_file.bin", std::ios::binary | std::ios::trunc);
  file.close();
  FileTape tape("test_tape_file.bin");
  int32_t value = 42;

  tape.write(value);
  EXPECT_FALSE(tape.isEnd());

  tape.rewind();
  EXPECT_FALSE(tape.isEnd());

  EXPECT_EQ(tape.read(), value);
  EXPECT_FALSE(tape.isEnd());

  tape.moveForward();
  EXPECT_TRUE(tape.isEnd());
}

TEST_F(FileTapeTest, IO_Sequential) {
  std::ofstream file("test_tape_file.bin", std::ios::binary | std::ios::trunc);
  file.close();
  FileTape tape("test_tape_file.bin");
  std::vector<int32_t> input = {10, 20, 30, 40, 50};

  for (int32_t v : input) {
    tape.write(v);
    tape.moveForward();
  }

  tape.rewind();

  for (size_t i = 0; i < input.size(); ++i) {
    EXPECT_EQ(tape.read(), input[i]) << "Mismatch at index " << i;
    tape.moveForward();
  }
  EXPECT_TRUE(tape.isEnd());
}

TEST_F(FileTapeTest, IO_BoundaryValues) {
  std::ofstream file("test_tape_file.bin", std::ios::binary | std::ios::trunc);
  file.close();
  FileTape tape("test_tape_file.bin");
  std::vector<int32_t> input = {
      std::numeric_limits<int32_t>::min(),
      1,
      std::numeric_limits<int32_t>::max(),
      2147483647,
  };

  for (int32_t value : input) {
    tape.write(value);
    tape.moveForward();
  }

  tape.rewind();

  for (uint32_t expected : input) {
    EXPECT_EQ(tape.read(), expected);
    tape.moveForward();
  }
  EXPECT_TRUE(tape.isEnd());
}

TEST_F(FileTapeTest, Nav_Move) {
  std::ofstream file("test_tape_file.bin", std::ios::binary | std::ios::trunc);
  file.close();
  FileTape tape("test_tape_file.bin");
  tape.write(100);
  tape.moveForward();
  tape.write(200);
  tape.moveForward();
  tape.write(300);
  tape.moveForward();

  tape.rewind();
  tape.moveForward();

  EXPECT_EQ(tape.read(), 200);

  tape.moveBackward();
  tape.moveBackward();

  EXPECT_EQ(tape.read(), 100);
}

TEST_F(FileTapeTest, Nav_OverwriteCurrent) {
  std::ofstream file("test_tape_file.bin", std::ios::binary | std::ios::trunc);
  file.close();
  FileTape tape("test_tape_file.bin");
  tape.write(10);
  tape.moveForward();
  tape.write(20);
  tape.moveForward();
  tape.write(30);
  tape.moveForward();

  tape.rewind();
  tape.moveForward();
  tape.write(99);

  EXPECT_FALSE(tape.isEnd());

  tape.rewind();
  EXPECT_EQ(tape.read(), 10);
  tape.moveForward();
  EXPECT_EQ(tape.read(), 99);
  tape.moveForward();
  EXPECT_EQ(tape.read(), 30);
  tape.moveForward();
  EXPECT_TRUE(tape.isEnd());
}

TEST_F(FileTapeTest, Nav_OverwriteNext) {
  std::ofstream file("test_tape_file.bin", std::ios::binary | std::ios::trunc);
  file.close();
  FileTape tape("test_tape_file.bin");
  tape.write(1);
  tape.moveForward();
  tape.write(2);
  tape.moveForward();
  tape.write(3);
  tape.moveForward();

  tape.rewind();
  EXPECT_EQ(tape.read(), 1);
  tape.moveForward();
  tape.write(42);

  tape.rewind();
  EXPECT_EQ(tape.read(), 1);
  tape.moveForward();
  EXPECT_EQ(tape.read(), 42);
  tape.moveForward();
  EXPECT_EQ(tape.read(), 3);
  tape.moveForward();
  EXPECT_TRUE(tape.isEnd());
}

TEST_F(FileTapeTest, Nav_EndLogic) {
  std::ofstream file("test_tape_file.bin", std::ios::binary | std::ios::trunc);
  file.close();
  FileTape tape("test_tape_file.bin");
  const FileTape &const_tape = tape;
  EXPECT_TRUE(tape.isEnd());
  EXPECT_TRUE(const_tape.isEnd());

  tape.write(1);
  EXPECT_FALSE(tape.isEnd());

  tape.rewind();
  EXPECT_FALSE(tape.isEnd());

  tape.read();
  EXPECT_FALSE(tape.isEnd());

  tape.moveForward();
  EXPECT_TRUE(tape.isEnd());
}

TEST_F(FileTapeTest, IO_Persistence) {
  {
    std::ofstream file("test_tape_file.bin",
                       std::ios::binary | std::ios::trunc);
    file.close();
    FileTape tape("test_tape_file.bin");
    tape.write(777);
    tape.moveForward();
    tape.write(888);
    tape.moveForward();
  }

  FileTape tape_new("test_tape_file.bin");
  EXPECT_FALSE(tape_new.isEnd());
  EXPECT_EQ(tape_new.read(), 777);
  tape_new.moveForward();
  EXPECT_EQ(tape_new.read(), 888);
  tape_new.moveForward();
  EXPECT_TRUE(tape_new.isEnd());
}

TEST_F(FileTapeTest, Nav_OverwritePersist) {
  {
    std::ofstream file("test_tape_file.bin",
                       std::ios::binary | std::ios::trunc);
    file.close();
    FileTape tape("test_tape_file.bin");
    tape.write(5);
    tape.moveForward();
    tape.write(6);
    tape.moveForward();
    tape.write(7);
    tape.moveForward();

    tape.rewind();
    tape.moveForward();
    tape.write(60);
  }

  FileTape tape("test_tape_file.bin");
  EXPECT_EQ(tape.read(), 5);
  tape.moveForward();
  EXPECT_EQ(tape.read(), 60);
  tape.moveForward();
  EXPECT_EQ(tape.read(), 7);
  tape.moveForward();
  EXPECT_TRUE(tape.isEnd());
}

TEST_F(FileTapeTest, Err_FileNotFound) {
  EXPECT_THROW(FileTape tape("absolute_chaos_no_file.bin"), std::runtime_error);
}

TEST_F(FileTapeTest, Err_ReadPastEnd) {
  std::ofstream file("test_tape_file.bin", std::ios::binary | std::ios::trunc);
  file.close();
  FileTape tape("test_tape_file.bin");
  tape.write(10);
  tape.moveForward();
  tape.rewind();
  tape.read();
  tape.moveForward();

  EXPECT_THROW(tape.read(), std::runtime_error);
}

TEST_F(FileTapeTest, Err_PartialValue) {
  {
    std::ofstream file("test_tape_file.bin",
                       std::ios::binary | std::ios::trunc);
    const int32_t value = 123456;
    const uint16_t partial = 77;
    file.write(reinterpret_cast<const char *>(&value), sizeof(value));
    file.write(reinterpret_cast<const char *>(&partial), sizeof(partial));
  }

  EXPECT_THROW(FileTape tape("test_tape_file.bin"), std::runtime_error);
}

TEST_F(FileTapeTest, Nav_BackwardAfterEnd) {
  std::ofstream file("test_tape_file.bin", std::ios::binary | std::ios::trunc);
  file.close();
  FileTape tape("test_tape_file.bin");
  tape.write(10);
  tape.moveForward();
  tape.write(20);
  tape.moveForward();
  tape.rewind();

  EXPECT_EQ(tape.read(), 10);
  tape.moveForward();
  EXPECT_EQ(tape.read(), 20);
  tape.moveForward();
  EXPECT_THROW(tape.read(), std::runtime_error);

  tape.moveBackward();
  EXPECT_EQ(tape.read(), 20);
  EXPECT_FALSE(tape.isEnd());
  tape.moveForward();
  EXPECT_TRUE(tape.isEnd());
}

TEST_F(FileTapeTest, Nav_ExtremeMove) {
  std::ofstream file("test_tape_file.bin", std::ios::binary | std::ios::trunc);
  file.close();
  FileTape tape("test_tape_file.bin");
  tape.write(10);
  tape.moveForward();
  tape.rewind();

  tape.moveBackward();
  tape.moveBackward();
  EXPECT_EQ(tape.read(), 10);

  tape.moveForward();
  tape.moveForward();
  EXPECT_TRUE(tape.isEnd());
}
