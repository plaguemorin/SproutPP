
#pragma once
#include <list>
#include <map>
#include <memory>
#include <string_view>
#include <system_error>

#include "sproutpp/property_resolver.h"

namespace framework::impl {

/**
   * Class CompositingPropertyResolver
   */
class CompositingPropertyResolver : public virtual PropertyResolver {
  std::map<std::string, PropertySource::Value, std::less<>> _staticSources;
  std::list<std::unique_ptr<PropertySource>> _sources;
  std::map<size_t, PropertySourceFactory> _property_source_factory;

  void valueChanged(std::string_view propertyName);
  std::string extractImportConfig(const PropertySource &source) const;
  void handleDynamicSourceNotifications(PropertySource &source);
  std::unique_ptr<PropertySource> loadPropertySourceFromImportString(std::string_view postImport);

protected:
  void registerPropertySourceFactory(std::string_view sourceName, PropertySourceFactory factory) override;

public:
  ~CompositingPropertyResolver() override = default;

  void registerPropertySource(std::unique_ptr<PropertySource> &&source) override;
  PropertySource::Value getProperty(std::string_view propertyName) override;
};

}// namespace framework::impl
