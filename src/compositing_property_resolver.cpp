
#include "compositing_property_resolver.h"

#include <ranges>

namespace framework::impl {

void CompositingPropertyResolver::valueChanged(std::string_view propertyName) {
}

void CompositingPropertyResolver::registerPropertySourceFactory(std::string_view sourceName, PropertySourceFactory factory) {
  _property_source_factory.try_emplace(std::hash<std::string_view>{}(sourceName), factory);
}

void CompositingPropertyResolver::registerPropertySource(std::unique_ptr<PropertySource> &&source) {
  std::string postImport;

  if (source->isStatic() && !source->hasValues()) {
    // If the source is static and has no values, don't bother doing
    // anything and just return silently
    return;
  }

  // Does this source have import lines?
  if (const auto importConfig = source->getProperty("import.config");
      std::holds_alternative<std::string>(importConfig)) {
    // Import should be in the form of, <sourceName>:<sourceParam>
    // Like file:/etc/config.properties
    // Or zookeeper:localhost:1234/config/rootName
    postImport = std::get<std::string>(importConfig);
  }

  // if this is a static source (the values cannot change)
  if (!source->isStatic()) {
    // Make sure we get notified of values changes so we can broadcast it
    source->clearValueWatchers();
    source->addValueWatcher([this](std::string_view propertyName) {
      valueChanged(propertyName);
    });
  }

  _sources.emplace_back(std::move(source));

  // Do we have a post-import ?
  if (!postImport.empty()) {
    loadPropertySource(postImport);
  }
}

PropertySource::Value CompositingPropertyResolver::getProperty(std::string_view propertyName) {
  // Search LIFO
  for (const auto &source: std::ranges::reverse_view(_sources)) {
    if (const auto value = source->getProperty(propertyName);
        !std::holds_alternative<std::monostate>(value)) {
      return value;
    }
  }

  return std::monostate();
}

std::error_code CompositingPropertyResolver::loadPropertySource(std::string_view propertySourceName) {
  // Find the first ":" to split on
  if (const auto delim = propertySourceName.find(':');
      delim != std::string::npos) {
    const auto sourceName = propertySourceName.substr(0, delim);
    const auto sourceParam = propertySourceName.substr(delim + 1);

    // Was this sourceName registered?
    if (const auto it = _property_source_factory.find(std::hash<std::string_view>{}(sourceName));
        it != _property_source_factory.end()) {
      registerPropertySource(it->second(sourceParam));
    }
  }

  return {};
}

}// namespace framework::impl
