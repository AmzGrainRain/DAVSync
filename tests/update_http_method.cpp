#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

enum class http_method {
  NIL = 0,
  GET,
  HEAD,
  POST,
  PUT,
  TRACE,
  PATCH,
  CONNECT,
  OPTIONS,
  DEL,
  PROPFIND,
  PROPPATCH,
  MKCOL,
  COPY,
  MOVE,
  LOCK,
  UNLOCK,
  // Non-Standard Methods
  ACL,
  SEARCH,
  REPORT
};

inline constexpr std::array<std::string_view, 19> method_string = {
    "GET",     "HEAD",    "POST", "PUT",      "TRACE",     "PATCH",
    "CONNECT", "OPTIONS", "DEL",  "PROPFIND", "PROPPATCH", "MKCOL",
    "COPY",    "MOVE",    "LOCK", "UNLOCK", "ACL", "SEARCH", "REPORT"};

// inline constexpr const char* method_string[] = {
//     "GET",     "HEAD",    "POST", "PUT",      "TRACE",     "PATCH",
//     "CONNECT", "OPTIONS", "DEL",  "PROPFIND", "PROPPATCH", "MKCOL",
//     "COPY",    "MOVE",    "LOCK", "UNLOCK"};

size_t MAP_SIZE = 20;

inline constexpr size_t get_hash(std::string_view mthd) {
  size_t hash = 0, i = 0, max = mthd.length();
  while (i < max) {
    hash += (mthd[i++] & ~0x20);
  }
  return hash % MAP_SIZE;
}

bool find() {
  uint8_t table[100] = {0};

  for (const auto method : method_string) {
    if (table[get_hash(method)] != 0) {
      return false;
    }
    table[get_hash(method)] += 1;
  }

  return true;
}

void find_min_map_size() {
  while (!find()) {
    MAP_SIZE += 1;
  }
}

int main() {
  // 寻找最优的 map 大小
  find_min_map_size();
  std::cout << "Perfect map size: " << MAP_SIZE << std::endl;

  std::vector<std::pair<uint8_t, uint8_t>> pairs;
  size_t i = 1;
  for (const auto method : method_string) {
    pairs.push_back({get_hash(method), i++});
  }

  std::sort(
      pairs.begin(), pairs.end(),
      [](const std::pair<uint8_t, uint8_t> &a,
         const std::pair<uint8_t, uint8_t> &b) { return a.first < b.first; });

  // 找到对应关系
  std::cout << "Map is:" << std::endl;
  uint8_t max = pairs[pairs.size() - 1].first;
  for (size_t i = 0; i <= max; ++i) {
    auto it = std::find_if(
        pairs.begin(), pairs.end(),
        [i](const std::pair<uint8_t, uint8_t> &p) { return p.first == i; });
    if (it == pairs.end()) {
      std::cout << "0, ";
      continue;
    }
    std::cout << (int)((*it).second) << ", ";
  }
  std::cout << std::endl;

  return 0;
}
