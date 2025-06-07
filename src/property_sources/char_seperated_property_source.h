
#pragma once
#include "map_property_source.h"

namespace framework::impl {

/**
 * Class CharSeperatedPropertySource
 */
class CharSeperatedPropertySource : public MapPropertySource {

public:
  explicit CharSeperatedPropertySource(std::string_view values);
  CharSeperatedPropertySource(std::string_view values, char separator);
  ~CharSeperatedPropertySource() override = default;

  bool isStatic() const override { return true; }
};

}// namespace framework::impl
