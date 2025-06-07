
#include "default_bean_factory_impl.h"
#include <algorithm>
#include <fmt/format.h>

namespace {
  std::string EMPTY_STRING{};
}

namespace framework::impl {
  bool DefaultBeanFactoryImpl::registerBean(const boost::typeindex::ctti_type_index &type, void *bean,
                                            deleter_t deleter, std::string &&name) {
    // Make sure the name is unique
    auto findname = std::ranges::find_if(_bean_holder, [name](const auto &holder) { return holder.name == name; });

    if (findname != _bean_holder.end()) {
      // Name conflict
      return false;
    }

    _bean_holder.emplace_back(bean, deleter, type, std::move(name));
    return true;
  }

  void DefaultBeanFactoryImpl::deleteBean(void *bean) {
    // Find the bean
    auto findbean = std::ranges::find_if(_bean_holder, [bean](const auto &holder) { return holder.bean == bean; });

    if (findbean != _bean_holder.end()) {
      _bean_holder.erase(findbean);
    }
  }

  void *DefaultBeanFactoryImpl::getFirstBeanOfType(const BeanType &type) {
    const auto it = std::ranges::find_if(_bean_holder, [type](const auto &holder) { return holder.type == type; });
    if (it != _bean_holder.end()) {
      return it->bean;
    }
    return nullptr;
  }

  std::string DefaultBeanFactoryImpl::makeDefaultBeanName(const BeanType &type) {
    auto count = std::ranges::count_if(_bean_holder, [type](const auto &holder) { return holder.type == type; });

    return fmt::format("{}_{}", type.name(), count);
  }

  void *DefaultBeanFactoryImpl::getBeanTypeByName(const BeanType &type, std::string_view view) {
    const auto it = std::ranges::find_if(
        _bean_holder, [type, view](const auto &holder) { return holder.type == type && holder.name == view; });

    if (it != _bean_holder.end()) {
      return it->bean;
    }

    return nullptr;
  }

  bool DefaultBeanFactoryImpl::isBeanKnown(void *beanPtr) const {
    return std::ranges::any_of(_bean_holder, [beanPtr](const auto &holder) { return holder.bean == beanPtr; });
  }

  BeanFactory::BeanNameT DefaultBeanFactoryImpl::beanName(void *beanPtr) const {
    const auto it =
        std::ranges::find_if(_bean_holder, [beanPtr](const auto &holder) { return holder.bean == beanPtr; });

    if (it != _bean_holder.end()) {
      return it->name;
    }

    return EMPTY_STRING;
  }


} // namespace framework::impl
