
#pragma once

#include <set>
#include <spdlog/spdlog.h>

#include "compositing_property_resolver.h"
#include "default_bean_factory_impl.h"
#include "sproutpp/application_context.h"

namespace framework::impl {
/**
 * Class DefaultApplicationContext
 */
class DefaultApplicationContext : public ApplicationContext,
                                  public CompositingPropertyResolver,
                                  public DefaultBeanFactoryImpl {
  std::shared_ptr<spdlog::logger> _my_logger;
  std::string _name;
  std::set<std::string> _enabledProfiles;

protected:
  void applicationContextAwareCreated(ApplicationContextAware *aware) override;

public:
  explicit DefaultApplicationContext(std::string name);
  ~DefaultApplicationContext() override;

  void initialize() override;
  ApplicationContext *getParentContext() const override { return nullptr; }
  void addActiveProfile(std::string profile) { _enabledProfiles.emplace(std::move(profile)); }
  std::string_view name() const override { return _name; }
  const std::set<std::string> &activeProfiles() const override { return _enabledProfiles; }
};

}// namespace framework::impl
