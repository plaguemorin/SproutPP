
#pragma once

#include "bean_factory.h"
#include "property_resolver.h"

#include <algorithm>
#include <set>

namespace framework {

/**
 * Class ApplicationContext
 */
class ApplicationContext : public virtual BeanFactory,
                           public virtual PropertyResolver {
public:
  static std::shared_ptr<ApplicationContext> Create(int argc, char **argv);

  ~ApplicationContext() override = default;
  
  virtual ApplicationContext *getParentContext() const = 0;

  /**
   * \brief Initializes the context with discovered values
   *
   * Once all the bootstrap config is loaded, and the property source factories are
   * registered; this method will load all the application.properites (including the
   * ones for each active profile)
   *
   * Can only be called if getState returns State::INITIALIZING
   *
   * This method can abort if the context failed to load
   */
  virtual void initialize() = 0;

  /**
   * \breif Return a name for the deployed application that this context belongs to.
   * 
   * @return a name for the deployed application can be an empty string
   */
  virtual std::string_view name() const = 0;

  virtual const std::set<std::string> &activeProfiles() const = 0;

  virtual bool isProfileEnabled(std::string_view profileName) const {
    return std::ranges::any_of(activeProfiles(), [profileName](const auto &enabledProfile) {
      return enabledProfile == profileName;
    });
  }
};

}// namespace framework
