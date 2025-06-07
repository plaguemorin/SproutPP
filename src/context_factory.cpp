
#include <CLI/CLI.hpp>

#include "default_application_context.h"
#include "property_sources/environment_property_source.h"
#include "property_sources/property_file_property_source.h"

namespace {
void extractActiveProfiles(
    const std::shared_ptr<framework::impl::DefaultApplicationContext> &context,
    const framework::PropertySource &property_source) {
  // Check which profiles we need to enable
  if (const auto profilesVariant = property_source.getProperty("profiles.active");
      std::holds_alternative<std::string>(profilesVariant)) {

    // Split on ',' adding each on to the `context`
    std::string_view profilesView{std::get<std::string>(profilesVariant)};
    size_t pos = 0;
    while ((pos = profilesView.find(',')) != std::string_view::npos) {
      context->addActiveProfile(std::string(profilesView.substr(0, pos)));
      profilesView = profilesView.substr(pos + 1);
    }

    if (!profilesView.empty()) {
      context->addActiveProfile(std::string(profilesView));
    }
  }
}

std::string extractApplicationName(const framework::PropertySource &property_source) {

  if (const auto nameVariant = property_source.getProperty("application.name");
      std::holds_alternative<std::string>(nameVariant)) {
    return std::get<std::string>(nameVariant);
  }

  return {};
}
}// namespace

namespace framework {
std::shared_ptr<ApplicationContext> ApplicationContext::Create(int argc, char **argv) {

  // Load initial environments
  impl::EnvironmentPropertySource env;
  impl::PropertyFilePropertySource bootstrapProperties("bootstrap.properties");

  // Figure out the app name
  std::string app_name = "<unknown>";
  app_name = extractApplicationName(bootstrapProperties);
  if (app_name.empty()) {
    app_name = extractApplicationName(env);
  }
  if (app_name.empty()) {
    if (argc > 0 && argv != nullptr && argv[0] != nullptr) {
      app_name = std::string{argv[0]};
    }
  }

  // Parse the arguments
  CLI::App app{"", app_name};
  app.add_option("--app-name", app_name, "Change the application name (takes precedence over the env and bootstrap.propeties file");

  std::vector<std::string> cli_active_profiles;
  app.add_option("--profile", cli_active_profiles, "Activate profiles");

  // Don't try and parse empty data
  if (argc > 0 && argv != nullptr) {
    argv = app.ensure_utf8(argv);
    app.parse(argc, argv);
  }

  // Create the context with defaults
  auto context = std::make_shared<impl::DefaultApplicationContext>(app_name);
  context->registerPropertySourceHandler<impl::PropertyFilePropertySource>("file");

  // Enable profiles
  extractActiveProfiles(context, env);
  extractActiveProfiles(context, bootstrapProperties);
  for (const auto& profile : cli_active_profiles) {
    context->addActiveProfile(profile);
  }

  return context;
}

}// namespace framework
