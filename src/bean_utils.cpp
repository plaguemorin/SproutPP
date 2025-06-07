
#include "sproutpp/bean_factory.h"

namespace framework {
std::string BeanFactory::makeDefaultBeanName(const BeanType &type) {
  auto name = std::string(type.name());
  return name;
}

}// namespace framework
