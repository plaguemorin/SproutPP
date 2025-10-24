
#include "environment_property_source.h"

#include <cstdlib>

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
EnvironmentPropertySource::EnvironmentPropertySource() {
  //
}

bool EnvironmentPropertySource::containsProperty(std::string_view propertyName) const {
  const auto name = makeName(propertyName);
  return std::getenv(name.c_str()) != nullptr;
}

PropertySource::Value EnvironmentPropertySource::getProperty(std::string_view propertyName) const {
  const auto name = makeName(propertyName);

  if (const auto rc = std::getenv(name.c_str())) {
    return std::string(rc);
  }

  return std::monostate{};
}
}// namespace framework::impl
