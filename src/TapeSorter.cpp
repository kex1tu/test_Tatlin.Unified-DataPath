#include "TapeSorter.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <functional>
// #include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <vector>

#include "FileTape.hpp"
#include "TapeConfig.hpp"
#include "TapeInterface.hpp"

struct ScopeExit {
  std::function<void()> fn;
  ~ScopeExit() { fn(); }
};

static constexpr uint32_t kMaxOpenTempTapes = 64;
static constexpr uint32_t kMinElementLimit = 1024;
void TapeSorter::Sort(TapeInterface& input, TapeInterface& output) {
  // Limit для RAM задан в байтах
  // считаем, что под хранение данных половина RAM
  // остальное под доп. расходы
  // так же будем считать что можно в оперативной памяти хранить не менее 1024
  // элемента

  const uint64_t kElementLimit = std::max<uint64_t>(
      kMinElementLimit, (FileTape::global_config.memory_limit_bytes / 11));

  // делим на sizeof(int32_t) чтобы узначть к-во элементов которое можно было бы
  // максимум разместить в памяти. делим на 2 т.к. radix sort требует доп.
  // память в размере N. и умножаем на 3/4 чтобы 1/4 использовать под доп
  // расходы а остальное под хранение данных
  //
  // получаем, что примерно 9% от общего объема памяти в байтах это к-во
  // элементов, для удобства делим на 11
  // ПО РЕЗУЛЬТАТАМ ТЕСТОВ минимальнный объём памяти требуемый для выполенения
  // программы < 10 мб, для удобства округляем до 16 мб

  /*const uint64_t kElementLimit = std::max<uint64_t>(
    kMinElementLimit,
    (((FileTape::global_config.memory_limit_bytes) / (2 * sizeof(int32_t))) *
     3) /
        4);*/

  const uint64_t tape_size = input.size();
  if (kElementLimit <= tape_size) {
    helpers::merge_sort(input, output, kElementLimit);
  } else {
    helpers::radix_sort(input, output);
  }
}
void TapeSorter::helpers::help_radix_sort(std::vector<int32_t>& input,
                                          std::vector<int32_t>& tmp) {
  if (tmp.size() != input.size()) {
    tmp.resize(input.size());
  }
  for (int byte = 0; byte < 4; ++byte) {
    std::array<uint64_t, 256> cnt{};
    for (uint32_t v : input) {
      uint32_t u = v ^ 0x80000000U;
      ++cnt[(u >> (byte * 8)) & 0xFF];
    }

    std::array<size_t, 256> start{};
    for (int b = 1; b < 256; ++b) {
      start[b] = start[b - 1] + cnt[b - 1];
    }

    for (int32_t v : input) {
      uint32_t u = static_cast<uint32_t>(v) ^ 0x80000000U;
      uint8_t b = (u >> (byte * 8)) & 0xFF;
      tmp[start[b]++] = v;
    }

    std::swap(input, tmp);
  }
}
void TapeSorter::helpers::radix_sort(TapeInterface& input,
                                     TapeInterface& output) {
  input.rewind();
  std::vector<int32_t> buf;
  buf.reserve(input.size());
  while (!input.isEnd()) {
    buf.push_back(input.read());
    input.moveForward();
  }
  std::vector<int32_t> tmp(buf.size());
  help_radix_sort(buf, tmp);
  output.rewind();
  for (int32_t v : buf) {
    output.write(v);
    output.moveForward();
  }
}

std::string merge_paths_to_file(std::vector<std::string>& paths,
                                uint64_t& counter) {
  /*if (paths.size() > kMaxOpenTempTapes) {
    std::cout << "merge_paths_to_file " << paths.size() << '\n';
  }*/

  std::string out_name = "tmp/level_merge_" + std::to_string(counter) + ".bin";
  ++counter;

  std::vector<std::unique_ptr<TapeInterface>> tapes;
  tapes.reserve(paths.size());
  std::transform(
      paths.begin(), paths.end(), std::back_inserter(tapes),
      [](const std::string& p) { return std::make_unique<FileTape>(p); });

  {
    FileTape out_tape(out_name, OpenMode::kCreate);
    TapeSorter::helpers::merge_tapes(tapes, out_tape);
  }

  tapes.clear();
  for (const auto& p : paths) {
    std::filesystem::remove(p);
  }

  paths.clear();
  return out_name;
}

void TapeSorter::helpers::merge_sort(TapeInterface& input,
                                     TapeInterface& output,
                                     uint64_t kElementLimit) {
  // std::cout << "kElementLimit " << kElementLimit << '\n';
  // std::cout << "tmp files " << input.size() / kElementLimit << '\n';
  std::filesystem::create_directories("tmp");
  ScopeExit cleanup{[]() { std::filesystem::remove_all("tmp"); }};

  uint64_t counter = 0;
  std::vector<std::vector<std::string>> levels;

  std::vector<int32_t> buffer;
  buffer.reserve(kElementLimit);
  std::vector<int32_t> tmp_radix(kElementLimit);

  input.rewind();
  while (!input.isEnd()) {
    buffer.clear();
    for (uint64_t i = 0; i < kElementLimit && !input.isEnd(); ++i) {
      buffer.push_back(input.read());
      input.moveForward();
    }
    help_radix_sort(buffer, tmp_radix);

    std::string filename = "tmp/chunk_" + std::to_string(counter++) + ".bin";
    {
      FileTape tmp_tape(filename, OpenMode::kCreate);
      for (int32_t v : buffer) {
        tmp_tape.write(v);
        tmp_tape.moveForward();
      }
    }

    if (levels.empty()) {
      levels.push_back({});
    }
    levels[0].push_back(filename);

    uint64_t curr_level = 0;

    while (levels[curr_level].size() >= kMaxOpenTempTapes) {
      if (curr_level > 1) {
        // std::cout << "level merging " << curr_level << '\n';
      }
      std::string merged_file =
          merge_paths_to_file(levels[curr_level], counter);

      if (levels.size() <= curr_level + 1) {
        levels.push_back({});
      }
      levels[curr_level + 1].push_back(merged_file);

      ++curr_level;
    }
  }

  for (size_t i = 0; i < levels.size() - 1; ++i) {
    if (levels[i].empty()) {
      continue;
    }
    if (i > 1) {
      // std::cout << "level merging late " << i << '\n';
    }

    std::string merged = merge_paths_to_file(levels[i], counter);
    levels[i + 1].push_back(merged);
  }

  std::string merged = merge_paths_to_file(levels[levels.size() - 1], counter);

  levels[levels.size() - 1].push_back(merged);
  std::string result_name;
  for (const auto& lvl : levels) {
    if (!lvl.empty()) {
      result_name = lvl[0];
    }
  }
  // std::cout << "levels " << levels.size() - 1 << '\n';
  FileTape result(result_name, OpenMode::kOpen);
  output.rewind();
  while (!result.isEnd()) {
    output.write(result.read());
    output.moveForward();
    result.moveForward();
  }
}

struct MyPair {
  int32_t value;
  uint64_t tape_index;
  bool operator>(const MyPair& other) const { return value > other.value; }
  bool operator<(const MyPair& other) const { return value < other.value; }
};
void TapeSorter::helpers::merge_tapes(
    std::vector<std::unique_ptr<TapeInterface>>& temp_tapes,
    TapeInterface& output) {
  std::vector<MyPair> pairs;
  pairs.reserve(temp_tapes.size());

  for (const auto& tape : temp_tapes) {
    tape->rewind();
  }
  for (uint64_t i = 0; i < temp_tapes.size(); ++i) {
    if (temp_tapes[i]->isEnd()) {
      continue;
    }
    pairs.emplace_back(temp_tapes[i]->read(), i);
    temp_tapes[i]->moveForward();
  }
  std::priority_queue<MyPair, std::vector<MyPair>, std::greater<MyPair>> heap(
      std::greater<MyPair>(), std::move(pairs));
  output.rewind();
  while (!heap.empty()) {
    MyPair min = heap.top();
    heap.pop();
    output.write(min.value);
    output.moveForward();
    uint64_t tape_index = min.tape_index;
    if (!temp_tapes[tape_index]->isEnd()) {
      heap.push({temp_tapes[tape_index]->read(), tape_index});
      temp_tapes[tape_index]->moveForward();
    }
  }
}