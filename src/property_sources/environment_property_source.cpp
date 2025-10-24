
#include "environment_property_source.h"

#include <cstdlib>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace {
std::string makeName(std::string_view name) {
  std::string result;

  // Replace '.' with '_'
  // Remove '-'
  // Make uppercase

  result.reserve(name.size());

  for (const auto c: name) {
    if (c == '.') {
      result += '_';
    } else if (c != '-') {
      result += static_cast<char>(std::toupper(c));
    }
  }

  return result;
}
}// namespace

namespace framework::impl {
bool EnvironmentPropertySource::containsProperty(std::string_view propertyName) const {
  const auto name = makeName(propertyName);
#ifdef _WIN32
  size_t requiredSize;
  getenv_s(&requiredSize, nullptr, 0, name.c_str());
  return requiredSize > 0;
#else
  return std::getenv(name.c_str()) != nullptr;
#endif
}

PropertySource::Value EnvironmentPropertySource::getProperty(std::string_view propertyName) const {
  const auto name = makeName(propertyName);

#ifdef _WIN32
  size_t requiredSize;
  getenv_s(&requiredSize, nullptr, 0, name.c_str());
  if (requiredSize > 0) {
    std::string result(requiredSize, '\0');
    getenv_s(&requiredSize, result.data(), result.size(), name.c_str());
    return result;
  }
#else
  if (const auto rc = std::getenv(name.c_str())) {
    return std::string(rc);
  }
#endif

  return std::monostate{};
}
}// namespace framework::impl
