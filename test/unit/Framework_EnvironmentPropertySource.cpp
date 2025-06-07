#include <catch2/catch_test_macros.hpp>

#include "property_sources/environment_property_source.h"

TEST_CASE("Read env to source") {
  setenv("TEST", "allo", 1);
  framework::impl::EnvironmentPropertySource propsource;

  REQUIRE(propsource.hasValues());
  REQUIRE(propsource.containsProperty("TEST"));
  REQUIRE(std::holds_alternative<std::string>(propsource.getProperty("TEST")));

  auto value = std::get<std::string>(propsource.getProperty("TEST"));
  REQUIRE(value == "allo");
}

