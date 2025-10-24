#include <catch2/catch_test_macros.hpp>

#include "property_sources/environment_property_source.h"

TEST_CASE("Read env to source") {
#ifdef _WIN32
  _putenv_s("TEST", "allo");
#else
  setenv("TEST", "allo", 1);
#endif
  framework::impl::EnvironmentPropertySource propsource;

  REQUIRE(propsource.hasValues());
  REQUIRE(propsource.containsProperty("TEST"));
  REQUIRE(std::holds_alternative<std::string>(propsource.getProperty("TEST")));

  auto value = std::get<std::string>(propsource.getProperty("TEST"));
  REQUIRE(value == "allo");
}

