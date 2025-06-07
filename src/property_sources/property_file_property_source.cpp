#include "property_file_property_source.h"
#include "utils.h"

#include <fstream>
#include <sstream>

namespace {
constexpr std::string_view COMMENT_PREFIX = ";#";
constexpr std::string_view WHITESPACE = " \t";
constexpr std::string_view NEXTLINE = " \\";
constexpr std::string_view VALUESPLUT = "=:";

constexpr bool isBom(std::string_view str) {
  return str.length() >= 3 &&
         static_cast<unsigned char>(str[0]) == 0xEF &&
         static_cast<unsigned char>(str[1]) == 0xBB &&
         static_cast<unsigned char>(str[2]) == 0xBF;
}

std::string_view trim(std::string_view str) {
  const auto start = str.find_first_not_of(WHITESPACE);
  if (start == std::string_view::npos) return {};

  const auto end = str.find_last_not_of(WHITESPACE);
  const auto len = end - start + 1;

  return str.substr(start, len);
}

std::pair<std::string, std::string> split(std::string_view str) {
  const auto split = str.find_first_of(VALUESPLUT);
  if (split == std::string_view::npos) return {};

  return std::make_pair(
      std::string(trim(str.substr(0, split))),
      std::string(trim(str.substr(split + 1))));
}

void completeLine(std::stringstream &ss, framework::impl::PropertyFilePropertySource &property_source) {
  auto [fst, snd] = split(ss.view());
  ss = std::stringstream();

  if (fst.empty()) return;
  if (snd.empty()) return;

  property_source.setProperty(fst, snd);
}

template<typename Stream>
void readPropertyFromStream(Stream &stream, framework::impl::PropertyFilePropertySource &property_source) {
  size_t lineno = 0;
  std::string rawLine;

  // Support multi-line values
  std::stringstream ss;

  while (std::getline(stream, rawLine)) {
    lineno++;
    std::string_view line{rawLine};

    // Accept UTF BOM on the first line
    if (lineno == 1 && isBom(line)) {
      line = line.substr(3);
    }

    const auto trimmedLine = trim(line);

    // Is this a line comment?
    if (trimmedLine.find_first_of(COMMENT_PREFIX) == 0) {
      continue;
    }

    // Skip empty lines
    if (trimmedLine.empty()) {
      if (!ss.view().empty()) {
        // This is technically wrong
        completeLine(ss, property_source);
      }
      continue;
    }

    // Is this line ending with "\"?
    if (trimmedLine.ends_with(NEXTLINE)) {
      // Remove the trailing NEXTLINE
      ss << trim(trimmedLine.substr(0, trimmedLine.size() - NEXTLINE.size()));
      continue;
    }

    ss << trimmedLine;
    // then it needs to be name[=:]value
    completeLine(ss, property_source);
  }

  // Do we still have data in ss ?
  if (!ss.view().empty()) {
    // Error !
    completeLine(ss, property_source);
  }
}

}// namespace

namespace framework::impl {
PropertyFilePropertySource::PropertyFilePropertySource(std::string_view path) {

  std::ifstream file{std::string(path)};
  if (file.is_open()) {
    readPropertyFromStream(file, *this);
  }
}
}// namespace framework::impl
