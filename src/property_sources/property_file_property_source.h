#pragma once
#include "map_property_source.h"

namespace framework::impl {
/**
 * Class PropertyFilePropertySource
 */
class PropertyFilePropertySource : public MapPropertySource {
public:
  explicit PropertyFilePropertySource(std::string_view path);
  ~PropertyFilePropertySource() override = default;

  bool isStatic() const override { return true; }
};

}// namespace framework::impl
