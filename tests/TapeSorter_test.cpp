#include "../src/TapeSorter.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <limits>
#include <random>
#include <vector>

#include "../src/FileTape.hpp"

class TapeSorterTest : public ::testing::Test {
 protected:
  static const std::string kInputFileName;
  static const std::string kOutputFileName;

  static void WriteDataToTape(const std::vector<int32_t>& data,
                              const std::string& filename) {
    FileTape tape(filename, OpenMode::kCreate);
    for (int32_t v : data) {
      tape.write(v);
      tape.moveForward();
    }
  }

  static std::vector<int32_t> ReadDataFromTape(const std::string& filename) {
    FileTape tape(filename);
    tape.rewind();
    std::vector<int32_t> data;
    while (!tape.isEnd()) {
      data.push_back(tape.read());
      tape.moveForward();
    }
    return data;
  }

  static bool IsSorted(const std::vector<int32_t>& data) {
    return std::is_sorted(data.begin(), data.end());
  }
};

const std::string TapeSorterTest::kInputFileName = "test_sort_input.bin";
const std::string TapeSorterTest::kOutputFileName = "test_sort_output.bin";

TEST_F(TapeSorterTest, RS_EmptyTape) {
  std::vector<int32_t> empty;
  WriteDataToTape(empty, kInputFileName);

  FileTape input(kInputFileName);
  FileTape output(kOutputFileName, OpenMode::kCreate);

  TapeSorter::Sort(input, output);

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  EXPECT_TRUE(result.empty());
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, RS_SingleElement) {
  std::vector<int32_t> data = {42};
  WriteDataToTape(data, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);
    TapeSorter::Sort(input, output);
    output.flush();
  }

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  EXPECT_EQ(result.size(), 1);
  EXPECT_EQ(result[0], 42);
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, RS_TwoElements_Sorted) {
  std::vector<int32_t> data = {10, 20};
  WriteDataToTape(data, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);
    TapeSorter::Sort(input, output);
    output.flush();
  }

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  EXPECT_EQ(result, data);
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, RS_TwoElements_Unsorted) {
  std::vector<int32_t> data = {20, 10};
  WriteDataToTape(data, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);
    TapeSorter::Sort(input, output);
    output.flush();
  }

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  std::vector<int32_t> expected = {10, 20};
  EXPECT_EQ(result, expected);
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, RS_AlreadySorted) {
  std::vector<int32_t> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  WriteDataToTape(data, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);
    TapeSorter::Sort(input, output);
    output.flush();
  }

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  EXPECT_EQ(result, data);
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, RS_BoundaryValues) {
  std::vector<int32_t> data = {std::numeric_limits<int32_t>::min(),
                               std::numeric_limits<int32_t>::max(), 0, 1, -1};
  WriteDataToTape(data, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);
    TapeSorter::Sort(input, output);
    output.flush();
  }

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  std::sort(data.begin(), data.end());
  EXPECT_EQ(result, data);
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, RS_NegativeNumbers) {
  std::vector<int32_t> data = {-100, -50, -1, -1000, -5};
  WriteDataToTape(data, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);
    TapeSorter::Sort(input, output);
    output.flush();
  }

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  std::vector<int32_t> expected = {-1000, -100, -50, -5, -1};
  EXPECT_EQ(result, expected);
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, RS_MixedSigns) {
  std::vector<int32_t> data = {-5, 10, -3, 8, 0, -1, 5, -10};
  WriteDataToTape(data, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);
    TapeSorter::Sort(input, output);
    output.flush();
  }

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  std::vector<int32_t> expected = {-10, -5, -3, -1, 0, 5, 8, 10};
  EXPECT_EQ(result, expected);
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, RS_PowersOfTwo) {
  std::vector<int32_t> data = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};
  WriteDataToTape(data, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);
    TapeSorter::Sort(input, output);
    output.flush();
  }

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  EXPECT_EQ(result, data);
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, RS_SmallDataset) {
  std::vector<int32_t> data(25);
  for (int i = 0; i < 25; ++i) {
    data[i] = (rand() % 1000);
  }
  WriteDataToTape(data, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);
    TapeSorter::Sort(input, output);
    output.flush();
  }

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  std::sort(data.begin(), data.end());
  EXPECT_EQ(result, data);
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, RS_MediumDataset) {
  std::vector<int32_t> data(1000);
  for (int i = 0; i < 1000; ++i) {
    data[i] = (rand() % 10000);
  }
  WriteDataToTape(data, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);
    TapeSorter::Sort(input, output);
    output.flush();
  }

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  std::sort(data.begin(), data.end());
  EXPECT_EQ(result, data);
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, RS_LargeDataset) {
  std::vector<int32_t> data(10000);
  for (int i = 0; i < 10000; ++i) {
    data[i] = (rand() % 100000);
  }
  WriteDataToTape(data, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);
    TapeSorter::Sort(input, output);
    output.flush();
  }

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  std::sort(data.begin(), data.end());
  EXPECT_EQ(result, data);
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, RS_SameBytes) {
  std::vector<int32_t> data = {0x01010101, 0x02020202, 0x03030303, 0x04040404};
  WriteDataToTape(data, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);
    TapeSorter::Sort(input, output);
    output.flush();
  }

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  std::vector<int32_t> expected = {0x01010101, 0x02020202, 0x03030303,
                                   0x04040404};
  EXPECT_EQ(result, expected);
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, RS_ReverseSorted) {
  std::vector<int32_t> data(100);
  for (int i = 99; i >= 0; --i) {
    data[i] = (100 - i);
  }
  WriteDataToTape(data, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);
    TapeSorter::Sort(input, output);
    output.flush();
  }

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  std::reverse(data.begin(), data.end());
  EXPECT_EQ(result, data);
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, RS_AllDuplicates) {
  std::vector<int32_t> data(50, 42);
  WriteDataToTape(data, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);
    TapeSorter::Sort(input, output);
    output.flush();
  }

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  EXPECT_EQ(result.size(), 50);
  for (int32_t v : result) {
    EXPECT_EQ(v, 42);
  }
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, RS_SomeDuplicates) {
  std::vector<int32_t> data = {5, 2, 5, 1, 2, 3, 5, 1, 2};
  WriteDataToTape(data, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);
    TapeSorter::Sort(input, output);
    output.flush();
  }

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  std::vector<int32_t> expected = {1, 1, 2, 2, 2, 3, 5, 5, 5};
  EXPECT_EQ(result, expected);
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, RS_TapeIntegration) {
  std::vector<int32_t> data = {100, -50, 0, 75, -25, 50, -100, 25};
  WriteDataToTape(data, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);

    EXPECT_EQ(input.size(), 8);
    EXPECT_EQ(output.size(), 0);

    TapeSorter::Sort(input, output);

    EXPECT_EQ(output.size(), 8);
    output.flush();
  }

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  std::vector<int32_t> expected = {-100, -50, -25, 0, 25, 50, 75, 100};
  EXPECT_EQ(result, expected);
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, RS_Persistence) {
  std::vector<int32_t> data = {10, 5, 8, 3, 1};
  WriteDataToTape(data, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);
    TapeSorter::Sort(input, output);
  }

  FileTape output(kOutputFileName);
  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  std::vector<int32_t> expected = {1, 3, 5, 8, 10};
  EXPECT_EQ(result, expected);
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, RS_MultipleRuns) {
  std::vector<int32_t> data1 = {3, 1, 2};
  std::vector<int32_t> data2 = {6, 4, 5};

  WriteDataToTape(data1, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);
    TapeSorter::Sort(input, output);
  }

  WriteDataToTape(data2, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);
    TapeSorter::Sort(input, output);
  }

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  std::vector<int32_t> expected = {4, 5, 6};
  EXPECT_EQ(result, expected);
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, RS_RandomSmall) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int32_t> dist(-1000, 1000);

  std::vector<int32_t> data(50);
  for (int i = 0; i < 50; ++i) {
    data[i] = (dist(gen));
  }
  WriteDataToTape(data, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);
    TapeSorter::Sort(input, output);
    output.flush();
  }

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  std::sort(data.begin(), data.end());
  EXPECT_EQ(result, data);
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, RS_RandomLarge) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int32_t> dist(-100000, 100000);

  std::vector<int32_t> data(5000);
  for (int i = 0; i < 5000; ++i) {
    data[i] = (dist(gen));
  }
  WriteDataToTape(data, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);
    TapeSorter::Sort(input, output);
    output.flush();
  }

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  std::sort(data.begin(), data.end());
  EXPECT_EQ(result, data);
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, RS_RandomFixedSeed) {
  std::mt19937 gen(42);
  std::uniform_int_distribution<int32_t> dist(-10000, 10000);

  std::vector<int32_t> data(100);
  for (int i = 0; i < 100; ++i) {
    data[i] = (dist(gen));
  }
  WriteDataToTape(data, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);
    TapeSorter::Sort(input, output);
    output.flush();
  }

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  std::sort(data.begin(), data.end());
  EXPECT_EQ(result, data);
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, MS_EmptyTape) {
  std::vector<int32_t> empty;
  WriteDataToTape(empty, kInputFileName);

  FileTape input(kInputFileName);
  FileTape output(kOutputFileName, OpenMode::kCreate);

  TapeSorter::Sort(input, output);

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  EXPECT_TRUE(result.empty());
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, MS_SingleElement) {
  std::vector<int32_t> data = {42};
  WriteDataToTape(data, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);
    TapeSorter::Sort(input, output);
    output.flush();
  }

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  EXPECT_EQ(result.size(), 1);
  EXPECT_EQ(result[0], 42);
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, MS_TwoElements) {
  std::vector<int32_t> data = {20, 10};
  WriteDataToTape(data, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);
    TapeSorter::Sort(input, output);
    output.flush();
  }

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  std::vector<int32_t> expected = {10, 20};
  EXPECT_EQ(result, expected);
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, MS_ExactlyRamLimit) {
  std::vector<int32_t> data(25);
  for (int i = 0; i < 25; ++i) {
    data[i] = (rand() % 1000);
  }
  WriteDataToTape(data, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);
    TapeSorter::Sort(input, output);
    output.flush();
  }

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  std::sort(data.begin(), data.end());
  EXPECT_EQ(result, data);
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, MS_JustAboveRamLimit) {
  std::vector<int32_t> data(26);
  for (int i = 0; i < 26; ++i) {
    data[i] = (rand() % 1000);
  }
  WriteDataToTape(data, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);
    TapeSorter::Sort(input, output);
    output.flush();
  }

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  std::sort(data.begin(), data.end());
  EXPECT_EQ(result, data);
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, MS_MultipleChunks) {
  std::vector<int32_t> data(500);
  for (int i = 0; i < 500; ++i) {
    data[i] = (rand() % 10000);
  }
  WriteDataToTape(data, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);
    TapeSorter::Sort(input, output);
    output.flush();
  }

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  std::sort(data.begin(), data.end());
  EXPECT_EQ(result, data);
  EXPECT_TRUE(IsSorted(result));
}

TEST_F(TapeSorterTest, MS_BoundaryValues) {
  std::vector<int32_t> data = {std::numeric_limits<int32_t>::min(),
                               std::numeric_limits<int32_t>::max(), 0, 1, -1};
  WriteDataToTape(data, kInputFileName);

  {
    FileTape input(kInputFileName);
    FileTape output(kOutputFileName, OpenMode::kCreate);
    TapeSorter::Sort(input, output);
    output.flush();
  }

  std::vector<int32_t> result = ReadDataFromTape(kOutputFileName);
  std::sort(data.begin(), data.end());
  EXPECT_EQ(result, data);
  EXPECT_TRUE(IsSorted(result));
}
