
#pragma once

namespace framework {
class BeanFactory;

/**
 * Class BeanFactoryAware
 */
class BeanFactoryAware {

public:
  virtual ~BeanFactoryAware() = default;
  virtual void setBeanFactory(BeanFactory &) = 0;
};

}// namespace framework
