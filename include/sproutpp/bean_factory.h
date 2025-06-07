
#pragma once

#include <functional>
#include <memory>

#include <boost/type_index/ctti_type_index.hpp>

#include "application_context_aware.h"
#include "bean_factory_aware.h"
#include "bean_name_aware.h"

namespace framework {

/**
 * Interface of the lightweight IoC container used across the framework.
 *
 * BeanFactory owns the lifetime of all beans registered within it and provides:
 * - Construction of singleton beans (createSingleton),
 * - Registration of externally constructed instances (registerExistingBean),
 * - Lookup by name and by type (getBeanTyped, getFirstBeanTyped),
 * - Hooking of environment-aware interfaces (ApplicationContextAware, BeanFactoryAware).
 *
 * Notes on ownership and destruction order:
 * - For beans constructed/registered through this factory, the factory is responsible for calling the
 *   correct deleter when the concrete implementation decides to destroy the bean. The concrete
 *   implementation must invoke deleteBean(void*) prior to actually destroying object memory, so that
 *   the allocator-aware destroy() can be called afterwards by the generated deleter.
 * - The actual storage and mapping between types/names is implementation-defined and provided by
 *   a concrete subclass.
 */
class BeanFactory {
protected:
  /// Type-erased deleter used by the factory to destroy stored beans.
  using deleter_t = std::function<void(void *)>;
  /// Compact compile-time type identifier used as the key for type-based lookups.
  using BeanType = boost::typeindex::ctti_type_index;

  /**
   * Hook invoked when a bean implementing ApplicationContextAware is created/registered.
   * Implementations should use this callback to inject the ApplicationContext into the bean.
   *
   * \param aware pointer to the bean that is ApplicationContextAware (non-owning).
   */
  virtual void applicationContextAwareCreated(ApplicationContextAware *aware) = 0;

  /**
   * Implementation-defined destruction bookkeeping for a bean instance.
   *
   * This function is called by the type-erased deleter before the object's destructor is invoked
   * (see makeDeleter). Concrete factories can use this to remove internal indices, break dependency
   * links, etc.
   *
   * \param bean raw pointer to the bean to be deleted (non-owning).
   */
  virtual void deleteBean(void *bean) = 0;

  /**
   * Registers a bean instance with the factory.
   *
   * \param type   the compile-time type id for the bean (cv removed).
   * \param bean   pointer to the storage where the bean will live; ownership transferred to factory.
   * \param deleter type-erased deleter to be used when removing the bean from the factory.
   * \param name   unique bean name within this factory. Implementations may enforce uniqueness.
   * \return true if the bean was successfully registered; false if a conflict or error occurred.
   */
  virtual bool registerBean(const BeanType &type, void *bean, deleter_t deleter, std::string &&name) = 0;

  /**
   * Looks up a bean by name and type.
   *
   * \param type expected type identifier to ensure type safety during retrieval.
   * \param view bean name to search for.
   * \return pointer to the bean if found, otherwise nullptr.
   */
  virtual void *getBeanTypeByName(const BeanType &type, std::string_view view) = 0;

  /**
   * Retrieves the first bean registered for a given type.
   *
   * \param type expected type identifier.
   * \return pointer to the bean if at least one exists, otherwise nullptr.
   */
  virtual void *getFirstBeanOfType(const BeanType &type) = 0;

  /**
   * Produces a default bean name for the given type.
   * Implementations may override customized naming strategies.
   *
   * \param type compile-time type identifier of the bean.
   * \return a default, implementation-defined, stable bean name for this type.
   */
  virtual std::string makeDefaultBeanName(const BeanType &type);

  /**
   * Creates a type-erased deleter for T that integrates with the factory's deletion pipeline.
   *
   * The deleter will first notify the factory via deleteBean(ptr) and then invoke the allocator-aware
   * destroy() for T on the provided memory address.
   *
   * \tparam Tp Bean type.
   * \return a callable suitable to be stored alongside the bean in the registry.
   */
  template<typename Tp>
  deleter_t makeDeleter() {
    return [this](void *ptr) {
      auto allocDeleter = std::allocator<Tp>();
      deleteBean(ptr);
      std::allocator_traits<decltype(allocDeleter)>::destroy(allocDeleter, static_cast<Tp *>(ptr));
    };
  }

  /**
   * Wires framework-aware dependencies into a freshly constructed/registered bean.
   *
   * - If Tp derives from ApplicationContextAware, notifies the factory to inject the context.
   * - If Tp derives from BeanFactoryAware, injects a reference to this factory.
   *
   * \tparam Tp Concrete bean type.
   * \param bean pointer to the newly available bean instance (non-owning).
   */
  template<typename Tp>
  void handleBeanDependencies(Tp *bean) {
    if constexpr (std::is_base_of_v<ApplicationContextAware, Tp>) {
      applicationContextAwareCreated(bean);
    }

    if constexpr (std::is_base_of_v<BeanFactoryAware, Tp>) {
      bean->setBeanFactory(*this);
    }

    if constexpr (std::is_base_of_v<BeanNameAware, Tp>) {
      bean->setBeanName(beanName(bean));
    }
  }

public:
  using BeanNameT = std::string_view;

  /// Virtual destructor to allow proper destruction through the interface.
  virtual ~BeanFactory() = default;

  /**
   * Creates and registers a singleton bean managed by the factory.
   *
   * Memory is allocated using std::allocator for Tp (with cv-qualifiers removed) and then the object
   * is constructed in-place. If registration fails (e.g., name conflict), the memory is deallocated
   * and nullptr is returned.
   *
   * \tparam Tp   Concrete bean type to create (must not be an array type).
   * \tparam Args Constructor argument types.
   * \param args  Constructor arguments forwarded to Tp's constructor.
   * \return pointer to the created bean on success; nullptr on failure.
   */
  template<typename Tp, typename... Args, std::enable_if_t<!std::is_array_v<Tp>, int> = 0>
  Tp *createSingleton(Args &&...args) {
    using TpNoCV = std::remove_cv_t<Tp>;
    auto type = BeanType::type_id<TpNoCV>();
    auto alloc = std::allocator<TpNoCV>();

    // Allocate memory
    if (auto ptr = std::allocator_traits<decltype(alloc)>::allocate(alloc, 1)) {
      // Register this bean
      if (!registerBean(type, ptr, makeDeleter<Tp>(), type.name())) {
        std::allocator_traits<decltype(alloc)>::deallocate(alloc, ptr, 1);
        return nullptr;
      }

      // Construct this bean
      std::allocator_traits<decltype(alloc)>::construct(alloc, ptr, std::forward<Args>(args)...);

      // Handle the interfaces that make this bean aware of its env
      handleBeanDependencies(ptr);
      return ptr;
    }

    return nullptr;
  }

  template<typename Tp>
  Tp *getNewInstance() {
    using TpNoCV = std::remove_cv_t<Tp>;
    auto type = BeanType::type_id<TpNoCV>();
    auto alloc = std::allocator<TpNoCV>();

    // Allocate memory
    if (auto ptr = std::allocator_traits<decltype(alloc)>::allocate(alloc, 1)) {
      // Register this bean
      if (!registerBean(type, ptr, makeDeleter<Tp>(), makeDefaultBeanName(type))) {
        std::allocator_traits<decltype(alloc)>::deallocate(alloc, ptr, 1);
        return nullptr;
      }

      // Construct this bean
      std::allocator_traits<decltype(alloc)>::construct(alloc, ptr);

      // Handle the interfaces that make this bean aware of its env
      handleBeanDependencies(ptr);
      return ptr;
    }

    return nullptr;
  }

  /**
   * Registers an already constructed bean instance with the factory.
   *
   * Ownership of the instance is transferred to the factory. If registration fails the function
   * will terminate the process (std::abort) because the factory cannot safely release ownership.
   * After successful registration, framework-aware dependencies are injected.
   *
   * \tparam Tp      Concrete bean type being registered.
   * \param ptr      unique_ptr owning the bean to register. Ownership is released on success.
   * \param beanName Optional explicit bean name. If empty, a default name is generated for Tp.
   * \return pointer to the registered bean on success; nullptr if the input pointer was empty.
   */
  template<typename Tp>
  Tp *registerExistingBean(std::unique_ptr<Tp> &&ptr, std::string_view beanName = "") {
    using TpNoCV = std::remove_cv_t<Tp>;
    auto type = BeanType::type_id<TpNoCV>();

    if (auto bean = ptr.release()) {
      // Register this bean with the factory
      if (!registerBean(type, bean, makeDeleter<Tp>(), (beanName.empty() ? makeDefaultBeanName(type) : std::string(beanName)))) {
        std::abort();
      }
      handleBeanDependencies<Tp>(bean);
      return bean;
    }

    return nullptr;
  }


  /**
   * Checks whether the factory currently knows/manages the given pointer.
   *
   * \param beanPtr raw pointer to a bean instance.
   * \return true if the bean pointer is currently registered; false otherwise.
   */
  virtual bool isBeanKnown(void *beanPtr) const = 0;

  /**
   * 
   * @param beanPtr the bean to lookup
   * @return the name of the bean or empty name if bean is not found
   */
  virtual BeanNameT beanName(void *beanPtr) const = 0;

  /**
   * Retrieves a bean by name and casts it to the requested type.
   *
   * \tparam Tp    Expected bean type.
   * \param beanName name used during registration.
   * \return pointer to the bean if found (or nullptr otherwise). No dynamic type check beyond
   *         the type id passed to the underlying implementation is performed here.
   */
  template<typename Tp>
  Tp *getBeanTyped(std::string_view beanName) {
    using TpNoCV = std::remove_cv_t<Tp>;
    return static_cast<Tp *>(getBeanTypeByName(BeanType::type_id<TpNoCV>(), beanName));
  }

  /**
   * Retrieves the first bean matching the requested type.
   *
   * \tparam Tp Expected bean type.
   * \return pointer to the first matching bean or nullptr if none exist.
   */
  template<typename Tp>
  Tp *getFirstBeanTyped() {
    using TpNoCV = std::remove_cv_t<Tp>;
    return static_cast<Tp *>(getFirstBeanOfType(BeanType::type_id<TpNoCV>()));
  }
};

}// namespace framework
