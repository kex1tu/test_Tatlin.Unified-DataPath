#pragma once
#include <cstdint>
#include <memory>
#include <vector>

#include "TapeInterface.hpp"

namespace TapeSorter {
void Sort(TapeInterface& input, TapeInterface& output);
namespace helpers {
void radix_sort(TapeInterface& input, TapeInterface& output);
void help_radix_sort(std::vector<int32_t>& input, std::vector<int32_t>& tmp);
void merge_sort(TapeInterface& input, TapeInterface& output,
                uint64_t kElementLimit);
void merge_tapes(std::vector<std::unique_ptr<TapeInterface>>& temp_tapes,
                 TapeInterface& output);
}  // namespace helpers
};  // namespace TapeSorter