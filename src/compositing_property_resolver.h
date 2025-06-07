
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
    std::map<std::string, PropertySource::Value> _staticSources;
    std::list<std::unique_ptr<PropertySource>> _sources;
    std::map<size_t, PropertySourceFactory> _property_source_factory;

    void valueChanged(std::string_view propertyName);

  protected:
    void registerPropertySourceFactory(std::string_view sourceName, PropertySourceFactory factory) override;

  public:
    ~CompositingPropertyResolver() override = default;

    void registerPropertySource(std::unique_ptr<PropertySource> &&source) override;
    PropertySource::Value getProperty(std::string_view propertyName) override;
    std::error_code loadPropertySource(std::string_view propertySourceName);
  };

} // namespace framework::impl
