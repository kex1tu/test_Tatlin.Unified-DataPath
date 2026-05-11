#include "../src/TapeConfig.hpp"

#include <gtest/gtest.h>

#include <fstream>
#include <string>

class TapeConfigTest : public ::testing::Test {
 protected:
  static void WriteConfigFile(const std::string& filename,
                              const std::string& content) {
    std::ofstream file(filename);
    file << content;
    file.close();
  }
};

TEST_F(TapeConfigTest, TC_DefaultValues) {
  TapeConfig config;
  EXPECT_EQ(config.memory_limit_bytes, 67108864);
  EXPECT_EQ(config.read_delay_ms, 0);
  EXPECT_EQ(config.write_delay_ms, 0);
  EXPECT_EQ(config.move_delay_ms, 0);
  EXPECT_EQ(config.rewind_delay_ms, 0);
}

TEST_F(TapeConfigTest, TC_LoadValidConfig) {
  std::string config_content =
      "memory_limit_bytes=134217728\n"
      "read_delay_ms=5\n"
      "write_delay_ms=10\n"
      "move_delay_ms=15\n"
      "rewind_delay_ms=20\n";

  WriteConfigFile("test_config.txt", config_content);

  TapeConfig config = TapeConfig::load("test_config.txt");
  EXPECT_EQ(config.memory_limit_bytes, 134217728);
  EXPECT_EQ(config.read_delay_ms, 5);
  EXPECT_EQ(config.write_delay_ms, 10);
  EXPECT_EQ(config.move_delay_ms, 15);
  EXPECT_EQ(config.rewind_delay_ms, 20);
}

TEST_F(TapeConfigTest, TC_ZeroDelaysConfig) {
  std::string config_content =
      "memory_limit_bytes=134217728\n"
      "read_delay_ms=0\n"
      "write_delay_ms=0\n"
      "move_delay_ms=0\n"
      "rewind_delay_ms=0\n";

  WriteConfigFile("test_config.txt", config_content);

  TapeConfig config = TapeConfig::load("test_config.txt");
  EXPECT_EQ(config.memory_limit_bytes, 134217728);
  EXPECT_EQ(config.read_delay_ms, 0);
  EXPECT_EQ(config.write_delay_ms, 0);
  EXPECT_EQ(config.move_delay_ms, 0);
  EXPECT_EQ(config.rewind_delay_ms, 0);
}

TEST_F(TapeConfigTest, TC_LoadNonExistentFile) {
  TapeConfig config = TapeConfig::load("non_existent_file.txt");

  EXPECT_EQ(config.memory_limit_bytes, 67108864);
  EXPECT_EQ(config.read_delay_ms, 0);
  EXPECT_EQ(config.write_delay_ms, 0);
  EXPECT_EQ(config.move_delay_ms, 0);
  EXPECT_EQ(config.rewind_delay_ms, 0);
}

TEST_F(TapeConfigTest, TC_KeyValueFormat) {
  std::string config_content =
      "memory_limit_bytes=268435456\n"
      "read_delay_ms=3\n"
      "write_delay_ms=7\n";

  WriteConfigFile("test_config.txt", config_content);

  TapeConfig config = TapeConfig::load("test_config.txt");
  EXPECT_EQ(config.memory_limit_bytes, 268435456);
  EXPECT_EQ(config.read_delay_ms, 3);
  EXPECT_EQ(config.write_delay_ms, 7);
  EXPECT_EQ(config.move_delay_ms, 0);
  EXPECT_EQ(config.rewind_delay_ms, 0);
}

TEST_F(TapeConfigTest, TC_EmptyFile) {
  WriteConfigFile("test_config.txt", "");

  TapeConfig config = TapeConfig::load("test_config.txt");

  EXPECT_EQ(config.memory_limit_bytes, 67108864);
  EXPECT_EQ(config.read_delay_ms, 0);
  EXPECT_EQ(config.write_delay_ms, 0);
  EXPECT_EQ(config.move_delay_ms, 0);
  EXPECT_EQ(config.rewind_delay_ms, 0);
}

TEST_F(TapeConfigTest, TC_MemoryLimit) {
  std::string config_content = "memory_limit_bytes=1073741824\n";
  WriteConfigFile("test_config.txt", config_content);

  TapeConfig config = TapeConfig::load("test_config.txt");
  EXPECT_EQ(config.memory_limit_bytes, 1073741824);
  EXPECT_EQ(config.read_delay_ms, 0);
  EXPECT_EQ(config.write_delay_ms, 0);
  EXPECT_EQ(config.move_delay_ms, 0);
  EXPECT_EQ(config.rewind_delay_ms, 0);
}

TEST_F(TapeConfigTest, TC_ReadDelay) {
  std::string config_content = "read_delay_ms=8\n";
  WriteConfigFile("test_config.txt", config_content);

  TapeConfig config = TapeConfig::load("test_config.txt");
  EXPECT_EQ(config.read_delay_ms, 8);
}

TEST_F(TapeConfigTest, TC_WriteDelay) {
  std::string config_content = "write_delay_ms=12\n";
  WriteConfigFile("test_config.txt", config_content);

  TapeConfig config = TapeConfig::load("test_config.txt");
  EXPECT_EQ(config.write_delay_ms, 12);
}

TEST_F(TapeConfigTest, TC_MoveDelay) {
  std::string config_content = "move_delay_ms=25\n";
  WriteConfigFile("test_config.txt", config_content);

  TapeConfig config = TapeConfig::load("test_config.txt");
  EXPECT_EQ(config.move_delay_ms, 25);
}

TEST_F(TapeConfigTest, TC_RewindDelay) {
  std::string config_content = "rewind_delay_ms=50\n";
  WriteConfigFile("test_config.txt", config_content);

  TapeConfig config = TapeConfig::load("test_config.txt");
  EXPECT_EQ(config.rewind_delay_ms, 50);
}

TEST_F(TapeConfigTest, TC_InvalidValues) {
  std::string config_content =
      "memory_limit_bytes=abc\n"
      "read_delay_ms=xyz\n";

  WriteConfigFile("test_config.txt", config_content);

  TapeConfig config = TapeConfig::load("test_config.txt");
  EXPECT_EQ(config.memory_limit_bytes, 67108864);
  EXPECT_EQ(config.read_delay_ms, 0);
}

TEST_F(TapeConfigTest, TC_UnknownKeys) {
  std::string config_content =
      "unknown_key=123\n"
      "another_unknown=456\n"
      "memory_limit_bytes=536870912\n";

  WriteConfigFile("test_config.txt", config_content);

  TapeConfig config = TapeConfig::load("test_config.txt");
  EXPECT_EQ(config.memory_limit_bytes, 536870912);
  EXPECT_EQ(config.read_delay_ms, 0);
  EXPECT_EQ(config.write_delay_ms, 0);
}

TEST_F(TapeConfigTest, TC_MissingKeys) {
  std::string config_content =
      "memory_limit_bytes=268435456\n"
      "read_delay_ms=4\n";

  WriteConfigFile("test_config.txt", config_content);

  TapeConfig config = TapeConfig::load("test_config.txt");
  EXPECT_EQ(config.memory_limit_bytes, 268435456);
  EXPECT_EQ(config.read_delay_ms, 4);
  EXPECT_EQ(config.write_delay_ms, 0);
  EXPECT_EQ(config.move_delay_ms, 0);
  EXPECT_EQ(config.rewind_delay_ms, 0);
}

TEST_F(TapeConfigTest, TC_NonNumericValues) {
  std::string config_content =
      "memory_limit_bytes=not_a_number\n"
      "read_delay_ms=also_not_a_number\n";

  WriteConfigFile("test_config.txt", config_content);

  TapeConfig config = TapeConfig::load("test_config.txt");
  EXPECT_EQ(config.memory_limit_bytes, 67108864);
  EXPECT_EQ(config.read_delay_ms, 0);
}

TEST_F(TapeConfigTest, TC_NegativeValues) {
  std::string config_content =
      "memory_limit_bytes=-1000\n"
      "read_delay_ms=-5\n";

  WriteConfigFile("test_config.txt", config_content);

  TapeConfig config = TapeConfig::load("test_config.txt");
  EXPECT_EQ(config.memory_limit_bytes, 67108864);
  EXPECT_EQ(config.read_delay_ms, 0);
}

TEST_F(TapeConfigTest, TC_ZeroValues) {
  std::string config_content =
      "memory_limit_bytes=0\n"
      "read_delay_ms=0\n"
      "write_delay_ms=0\n";

  WriteConfigFile("test_config.txt", config_content);

  TapeConfig config = TapeConfig::load("test_config.txt");
  EXPECT_EQ(config.memory_limit_bytes, 0);
  EXPECT_EQ(config.read_delay_ms, 0);
  EXPECT_EQ(config.write_delay_ms, 0);
  EXPECT_EQ(config.move_delay_ms, 0);
  EXPECT_EQ(config.rewind_delay_ms, 0);
}

TEST_F(TapeConfigTest, TC_MultipleLinesSameKey) {
  std::string config_content =
      "memory_limit_bytes=134217728\n"
      "memory_limit_bytes=268435456\n"
      "memory_limit_bytes=536870912\n";

  WriteConfigFile("test_config.txt", config_content);

  TapeConfig config = TapeConfig::load("test_config.txt");
  EXPECT_EQ(config.memory_limit_bytes, 536870912);
}

TEST_F(TapeConfigTest, TC_WhitespaceHandling) {
  std::string config_content =
      "  memory_limit_bytes = 134217728  \n"
      "\tread_delay_ms=5\n"
      "write_delay_ms = 10\n";

  WriteConfigFile("test_config.txt", config_content);

  TapeConfig config = TapeConfig::load("test_config.txt");
  EXPECT_EQ(config.memory_limit_bytes, 134217728);
  EXPECT_EQ(config.read_delay_ms, 5);
  EXPECT_EQ(config.write_delay_ms, 10);
}

TEST_F(TapeConfigTest, TC_CommentHandling) {
  std::string config_content =
      "# This is a comment\n"
      "memory_limit_bytes=134217728\n"
      "# Another comment\n"
      "read_delay_ms=5\n";

  WriteConfigFile("test_config.txt", config_content);

  TapeConfig config = TapeConfig::load("test_config.txt");
  EXPECT_EQ(config.memory_limit_bytes, 134217728);
  EXPECT_EQ(config.read_delay_ms, 5);
}
