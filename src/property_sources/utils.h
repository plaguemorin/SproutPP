#pragma once

#include <string>
#include <string_view>

namespace framework::impl {
inline std::string_view trim(const std::string_view &str) {
  const auto first = str.find_first_not_of(" \t\n\r");
  if (std::string_view::npos == first) {
    return str;
  }

  const auto last = str.find_last_not_of(" \t\n\r");
  return str.substr(first, (last - first + 1));
}

inline std::pair<std::string_view, std::string_view> makeKV(std::string_view str) {
  // Split at the =
  if (const auto delimiterPos = str.find('=');
      delimiterPos != std::string::npos) {
    auto key = trim(str.substr(0, delimiterPos));
    auto value = trim(str.substr(delimiterPos + 1));

    if (!key.empty() && !value.empty()) {
      std::make_pair(key, value);
    }
  }

  return {};
}

}// namespace framework::impl
