
#include "char_seperated_property_source.h"

#include "utils.h"

namespace framework::impl {
CharSeperatedPropertySource::CharSeperatedPropertySource(std::string_view values)
    : CharSeperatedPropertySource(values, ';') {
}

CharSeperatedPropertySource::CharSeperatedPropertySource(std::string_view values, char separator) {
  size_t start = 0;
  size_t end = values.find(separator);

  while (end != std::string::npos) {
    if (end > start) {
      if (const auto [fst, snd] = makeKV(values.substr(start, end - start));
          !fst.empty() && !snd.empty()) {
        setProperty(fst, std::string{snd});
      }
    }
    start = end + 1;
    end = values.find(separator, start);
  }

  if (start < values.length()) {
    if (const auto [fst, snd] = makeKV(values.substr(start));
      !fst.empty() && !snd.begin()) {
      setProperty(fst, std::string{snd});
    }
  }
}


}// namespace framework::impl
