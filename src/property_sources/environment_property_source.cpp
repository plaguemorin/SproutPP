
#include "environment_property_source.h"

#include <stdio.h>

#if defined(WIN) && (_MSC_VER >= 1900)
#else
extern char **environ;
#endif

namespace {
char **getEnv() {
#if defined(WIN) && (_MSC_VER >= 1900)
  return *::__p__environ();
#else
  return environ;
#endif
}

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
      result += std::toupper(c);
    }
  }

  return result;
}
}// namespace

namespace framework::impl {
EnvironmentPropertySource::EnvironmentPropertySource() {
  for (char **e = getEnv(); *e != nullptr; e++) {
    const auto thisEnv = std::string_view{*e};

    if (const auto eq = thisEnv.find('=');
        eq != std::string_view::npos) {
      setProperty(thisEnv.substr(0, eq), std::string{thisEnv.substr(eq + 1)});
    } else {
      setProperty(thisEnv, true);
    }
  }
}

bool EnvironmentPropertySource::containsProperty(std::string_view propertyName) const {
  return MapPropertySource::containsProperty(makeName(propertyName));
}

PropertySource::Value EnvironmentPropertySource::getProperty(std::string_view propertyName) const {
  return MapPropertySource::getProperty(makeName(propertyName));
}


}// namespace framework::impl
