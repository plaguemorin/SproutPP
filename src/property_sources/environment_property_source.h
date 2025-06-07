
#pragma once
#include "map_property_source.h"

namespace framework::impl {

/**
 * Class EnvironmentPropertySource
 */
class EnvironmentPropertySource : public MapPropertySource {
public:
  EnvironmentPropertySource();
  ~EnvironmentPropertySource() override = default;

  bool isStatic() const override { return true; }
  bool containsProperty(std::string_view propertyName) const override;
  Value getProperty(std::string_view propertyName) const override;
};

}// namespace framework::impl
