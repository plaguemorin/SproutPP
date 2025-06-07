
#pragma once
#include "sproutpp/bean_factory.h"

#include <list>

namespace framework::impl {

/**
 * Class DefaultBeanFactoryImpl
 */
class DefaultBeanFactoryImpl : public virtual BeanFactory {
  struct BeanHolder {
    void *bean;
    deleter_t deleter;
    boost::typeindex::ctti_type_index type;
    std::string name;
  };

  std::list<BeanHolder> _bean_holder;

protected:
  void deleteBean(void *bean) override;
  bool registerBean(const BeanType &type, void *bean, deleter_t deleter, std::string &&name) override;
  void *getBeanTypeByName(const BeanType &type, std::string_view view) override;
  void *getFirstBeanOfType(const BeanType &type) override;
  std::string makeDefaultBeanName(const BeanType &type) override;

public:
  ~DefaultBeanFactoryImpl() override = default;

  bool isBeanKnown(void *beanPtr) const override;
  BeanNameT beanName(void *beanPtr) const override;
};

}// namespace framework::impl
