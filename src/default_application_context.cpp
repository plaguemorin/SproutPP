
#include "default_application_context.h"

#include <fmt/format.h>

#include "property_sources/environment_property_source.h"
#include "property_sources/property_file_property_source.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace {
spdlog::sink_ptr make_default_sink() {
  return std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
}
}// namespace

namespace framework::impl {
DefaultApplicationContext::DefaultApplicationContext(std::string name)
    : _my_logger(std::make_shared<spdlog::logger>(name, make_default_sink())),
      _name(std::move(name)) {
  spdlog::initialize_logger(_my_logger);
  _my_logger->info("Creating application context {}", _name);
}

DefaultApplicationContext::~DefaultApplicationContext() {
  _my_logger->info("Destroying application context {}", _name);
}

void DefaultApplicationContext::initialize() {
  // Load application.properties and for each active profile load application-<profile>.properties
  registerPropertySource(std::make_unique<PropertyFilePropertySource>("application.properties"));

  for (const auto &activeProfile: activeProfiles()) {
    registerPropertySource(std::make_unique<PropertyFilePropertySource>(fmt::format("application-{}.properties", activeProfile)));
  }

  // Load ENV also
  registerPropertySource(std::make_unique<EnvironmentPropertySource>());
}

void DefaultApplicationContext::applicationContextAwareCreated(ApplicationContextAware *aware) {
  aware->setApplicationContext(*this);
}
}// namespace framework::impl
