
#pragma once

#include "sproutpp/property_source.h"

namespace framework::impl {

/**
 * Class EnvironmentPropertySource
 */
class EnvironmentPropertySource : public PropertySource {
public:
  EnvironmentPropertySource();
  ~EnvironmentPropertySource() override = default;

  bool containsProperty(std::string_view propertyName) const override;
  Value getProperty(std::string_view propertyName) const override;
  bool hasValues() const override { return true; }
};

}// namespace framework::impl
