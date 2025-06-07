#include <catch2/catch_test_macros.hpp>

#include <fstream>

#include "property_sources/property_file_property_source.h"
#include "sproutpp/property_source.h"

void writePropertiesFile() {
  std::ofstream file("test.properties", std::ios::out);
  file << "TEST=allo\nILO=world\n#this is a comment\n  # also=a comment\n\n\nMULTILINE=s \\\n\tad\nMULTISPACE=d  \\\ne sk";
  file.close();
}

TEST_CASE("Can read simple properties file") {
  writePropertiesFile();

  framework::impl::PropertyFilePropertySource propsource("test.properties");
  REQUIRE(propsource.hasValues());
  
  REQUIRE(propsource.containsProperty("TEST"));
  REQUIRE(std::holds_alternative<std::string>(propsource.getProperty("TEST")));
  REQUIRE(std::get<std::string>(propsource.getProperty("TEST")) == "allo");

  REQUIRE(propsource.containsProperty("ILO"));
  REQUIRE(std::holds_alternative<std::string>(propsource.getProperty("ILO")));
  REQUIRE(std::get<std::string>(propsource.getProperty("ILO")) == "world");

  REQUIRE(propsource.containsProperty("MULTILINE"));
  REQUIRE(std::holds_alternative<std::string>(propsource.getProperty("MULTILINE")));
  REQUIRE(std::get<std::string>(propsource.getProperty("MULTILINE")) == "sad");

  REQUIRE(propsource.containsProperty("MULTISPACE"));
  REQUIRE(std::holds_alternative<std::string>(propsource.getProperty("MULTISPACE")));
  REQUIRE(std::get<std::string>(propsource.getProperty("MULTISPACE")) == "de sk");
  
}
