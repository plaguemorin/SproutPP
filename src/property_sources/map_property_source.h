
#pragma once
#include <map>
#include <ranges>

#include "sproutpp/property_source.h"

namespace framework::impl {

/**
 * Class MapPropertySource
 */
class MapPropertySource : public PropertySource {
  using Key = decltype(std::hash<std::string_view>{}(""));// should be size_t
  std::map<Key, Value> _properties;

  static constexpr Key geyKey(std::string_view key) {
    return std::hash<std::string_view>{}(key);
  }

public:
  ~MapPropertySource() override = default;

  bool hasValues() const override { return !_properties.empty(); }
  
  bool isStatic() const override { return false; }

  bool containsProperty(std::string_view propertyName) const override {
    return _properties.contains(geyKey(propertyName));
  }

  Value getProperty(std::string_view propertyName) const override {
    const auto key = geyKey(propertyName);
    if (!_properties.contains(key)) {
      return std::monostate{};
    }
    return _properties.at(key);
  }

  void setProperty(std::string_view propertyName, const Value &value) {
    _properties.erase(geyKey(propertyName));
    _properties.emplace(geyKey(propertyName), value);
    notifyValueChanged(propertyName);
  }

  void removeProperty(std::string_view propertyName) {
    if (_properties.erase(geyKey(propertyName)) == 1) {
      notifyValueChanged(propertyName);
    }
  }
};

}// namespace framework::impl
