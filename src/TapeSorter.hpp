#pragma once
#include <cstdint>
#include <memory>
#include <vector>

#include "TapeInterface.hpp"

class TapeSorter {
 public:
  static void Sort(TapeInterface& input, TapeInterface& output);

 private:
  static void radix_sort(TapeInterface& input, TapeInterface& output);
  static void help_radix_sort(std::vector<int32_t>& input,
                              std::vector<int32_t>& output);
  static void merge_sort(TapeInterface& input, TapeInterface& output,
                         uint64_t kElementLimit);
  static void merge_tapes(
      std::vector<std::unique_ptr<TapeInterface>>& temp_tapes,
      TapeInterface& output);
};