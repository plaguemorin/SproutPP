
#pragma once

namespace framework {
class ApplicationContext;

/**
 * Class ApplicationContextAware
 */
class ApplicationContextAware {

public:
  virtual ~ApplicationContextAware() = default;
  virtual void setApplicationContext(ApplicationContext &) = 0;
};

}// namespace framework
