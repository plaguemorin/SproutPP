
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

  /**
   * FNV-1a 32/64bit algorithm
   * See https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function for more details
   * 
   * @param key the key to hash
   * @return hash
   */
  static constexpr Key geyKey(const std::string_view key) {
    static_assert(sizeof(size_t) == 8 || sizeof(size_t) == 4);

    size_t result;
    // FNV_offset_basis
    if constexpr (sizeof(size_t) == 8) {
      result = 0xcbf29ce484222325;
    } else {
      result = 0x811c9dc5;
    }

    for (const auto &character: key) {
      result ^= static_cast<size_t>(character);
      if constexpr (sizeof(size_t) == 8) {
        result *= 0x100000001b3; // FNV_prime
      } else {
        result *= 0x01000193;
      }
    }

    return result;
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
    _properties.try_emplace(geyKey(propertyName), value);
    notifyValueChanged(propertyName);
  }

  void removeProperty(std::string_view propertyName) {
    if (_properties.erase(geyKey(propertyName)) == 1) {
      notifyValueChanged(propertyName);
    }
  }
};

}// namespace framework::impl
