#include "TapeSorter.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <queue>
#include <string>

#include "FileTape.hpp"
#include "TapeConfig.hpp"
static constexpr uint64_t kMaxOpenTempTapes = 64;
static constexpr uint64_t kMinElementLimit = 1024;
void TapeSorter::Sort(TapeInterface& input, TapeInterface& output) {
  // Limit для RAM задан в байтах
  // считаем, что под хранение данных половина RAM
  // остальное под доп. расходы
  // так же будем считать что не менее 4 мб под хранение данных(1024 элемента)
  const uint64_t kElementLimit = std::max<uint64_t>(
      kMinElementLimit,
      FileTape::global_config.memory_limit_bytes / (2 * sizeof(int32_t)));

  const uint64_t tape_size = input.size();
  if (kElementLimit <= tape_size) {
    merge_sort(input, output, kElementLimit);
  } else {
    radix_sort(input, output);
  }
}
void TapeSorter::help_radix_sort(std::vector<int32_t>& input,
                                 std::vector<int32_t>& output) {
  std::vector<int32_t> tmp(input.size());

  for (int byte = 0; byte < 4; ++byte) {
    std::array<uint64_t, 256> cnt{};
    for (int32_t v : input) {
      uint32_t u = static_cast<uint32_t>(v) ^ 0x80000000U;
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
  output = input;
}
void TapeSorter::radix_sort(TapeInterface& input, TapeInterface& output) {
  input.rewind();
  std::vector<int32_t> buf;
  while (!input.isEnd()) {
    buf.push_back(input.read());
    input.moveForward();
  }
  help_radix_sort(buf, buf);
  output.rewind();
  for (int32_t v : buf) {
    output.write(v);
    output.moveForward();
  }
}

void TapeSorter::merge_sort(TapeInterface& input, TapeInterface& output,
                            uint64_t kElementLimit) {
  std::filesystem::create_directories("tmp");

  std::vector<int32_t> tmp(kElementLimit);
  uint64_t k = (input.size() / kElementLimit) +
               static_cast<uint64_t>(input.size() % kElementLimit != 0);

  std::vector<std::string> temp_files;
  temp_files.reserve(k);

  input.rewind();
  for (uint64_t i = 0; i < k; ++i) {
    tmp.clear();
    for (uint64_t j = 0; j < kElementLimit && !input.isEnd(); ++j) {
      tmp.push_back(input.read());
      input.moveForward();
    }
    help_radix_sort(tmp, tmp);

    std::string filename = "tmp/tmp_" + std::to_string(i) + ".bin";
    {
      auto tmp_tape = std::make_unique<FileTape>(filename, OpenMode::kCreate);
      for (int32_t v : tmp) {
        tmp_tape->write(v);
        tmp_tape->moveForward();
      }
    }
    temp_files.push_back(filename);
  }
  while (temp_files.size() > 1) {
    const uint64_t passes =
        (temp_files.size() + kMaxOpenTempTapes - 1) / kMaxOpenTempTapes;

    std::vector<std::string> temp_files_pass;
    temp_files_pass.reserve(passes);
    for (uint64_t i = 0; i < passes; ++i) {
      std::vector<std::unique_ptr<TapeInterface>> temp_tapes;
      temp_tapes.reserve(kMaxOpenTempTapes);

      for (uint64_t j = 0; j < kMaxOpenTempTapes &&
                           (i * kMaxOpenTempTapes) + j < temp_files.size();
           ++j) {
        temp_tapes.push_back(std::make_unique<FileTape>(
            temp_files[(i * kMaxOpenTempTapes) + j], OpenMode::kOpen));
      }

      std::string filename("tmp/tmp_" + std::to_string(temp_files.size()) +
                           "_" + std::to_string(passes) + "_" +
                           std::to_string(i) + ".bin");
      temp_files_pass.push_back(filename);
      {
        FileTape tmp_output(filename, OpenMode::kCreate);

        merge_tapes(temp_tapes, tmp_output);
      }
    }
    for (const auto& filename : temp_files) {
      if (std::filesystem::exists(filename)) {
        std::filesystem::remove(filename);
      }
    }
    std::swap(temp_files_pass, temp_files);
  }
  FileTape result(temp_files[0], OpenMode::kOpen);
  output.rewind();
  while (!result.isEnd()) {
    output.write(result.read());
    output.moveForward();
    result.moveForward();
  }
  for (const auto& filename : temp_files) {
    if (std::filesystem::exists(filename)) {
      std::filesystem::remove(filename);
    }
  }
}
void TapeSorter::merge_tapes(
    std::vector<std::unique_ptr<TapeInterface>>& temp_tapes,
    TapeInterface& output) {
  std::priority_queue<std::pair<int32_t, uint64_t>,
                      std::vector<std::pair<int32_t, uint64_t>>,
                      std::greater<std::pair<int32_t, uint64_t>>>
      heap;
  for (const auto& tape : temp_tapes) {
    tape->rewind();
  }
  for (uint64_t i = 0; i < temp_tapes.size(); ++i) {
    if (temp_tapes[i]->isEnd()) {
      continue;
    }
    heap.push({temp_tapes[i]->read(), i});
    temp_tapes[i]->moveForward();
  }
  output.rewind();
  while (!heap.empty()) {
    std::pair<int32_t, uint64_t> min = heap.top();
    heap.pop();
    output.write(min.first);
    output.moveForward();
    if (!temp_tapes[min.second]->isEnd()) {
      heap.push({temp_tapes[min.second]->read(), min.second});
      temp_tapes[min.second]->moveForward();
    }
  }
}