
#include "compositing_property_resolver.h"

#include <ranges>

namespace framework::impl {

void CompositingPropertyResolver::valueChanged(std::string_view propertyName) {
}

void CompositingPropertyResolver::registerPropertySourceFactory(std::string_view sourceName, PropertySourceFactory factory) {
  _property_source_factory.try_emplace(std::hash<std::string_view>{}(sourceName), factory);
}

std::string CompositingPropertyResolver::extractImportConfig(const PropertySource &source) const {
  // Does this source have import lines?
  // Import should be in the form of, <sourceName>:<sourceParam>
  // Like file:/etc/config.properties
  // Or zookeeper:localhost:1234/config/rootName
  const auto importConfig = source.getProperty("import.config");
  return std::holds_alternative<std::string>(importConfig)
             ? std::get<std::string>(importConfig)
             : std::string();
}

void CompositingPropertyResolver::handleDynamicSourceNotifications(PropertySource &source) {
  // if this is a static source (the values cannot change)
  if (!source.isStatic()) {
    // Make sure we get notified of values changes so we can broadcast it
    source.clearValueWatchers();
    source.addValueWatcher([this](std::string_view propertyName) {
      valueChanged(propertyName);
    });
  }
}

std::unique_ptr<PropertySource> CompositingPropertyResolver::loadPropertySourceFromImportString(std::string_view postImport) {
  // Do we have another source to load?
  if (const auto delim = postImport.find(':');
      delim != std::string::npos) {
    const auto sourceName = postImport.substr(0, delim);
    const auto sourceParam = postImport.substr(delim + 1);

    // Was this sourceName registered?
    if (const auto it = _property_source_factory.find(std::hash<std::string_view>{}(sourceName));
        it != _property_source_factory.end()) {
      return it->second(sourceParam);
    }
  }

  return nullptr;
}
void CompositingPropertyResolver::registerPropertySource(std::unique_ptr<PropertySource> &&source) {
  if (!source || (source->isStatic() && !source->hasValues())) {
    // If the source is static and has no values, don't bother doing
    // anything and just return silently
    return;
  }

  while (source) {
    const auto postImport = extractImportConfig(*source);
    handleDynamicSourceNotifications(*source);
    _sources.emplace_back(std::move(source));
    source = loadPropertySourceFromImportString(postImport);
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

}// namespace framework::impl
