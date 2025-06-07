
#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace framework {

/**
 * \brief Abstract source of configuration properties.
 *
 * PropertySource defines a common interface for components capable of supplying
 * configuration values. Concrete implementations may read from environment
 * variables, configuration files, key/value maps, remote services, etc.
 *
 * Key characteristics:
 * - Values are returned as a tagged union (variant) supporting string, int, double, bool,
 *   or std::monostate when a property is absent.
 * - Sources may be dynamic and notify listeners when values change, or static where
 *   values never change during the lifetime of the source.
 *
 * Thread-safety is not mandated by this interface. Implementations should document
 * any concurrency guarantees they provide.
 */
class PropertySource {
protected:
  /**
   * \brief Notify all registered watchers that a property's value changed.
   *
   * Implementations should call this helper when they detect that the value of
   * a property has changed. No notification is sent if the source is static
   * (isStatic() returns true).
   *
   * \param propertyName The name of the property that changed.
   */
  void notifyValueChanged(std::string_view propertyName) const {
    if (isStatic()) {
      return;
    }

    for (auto &callback: _callbacks) {
      callback(propertyName);
    }
  }

public:
  /**
   * \brief Variant type representing a property value.
   *
   * Holds one of: std::string, int, double, bool, or std::monostate when the
   * property is not present.
   */
  using Value = std::variant<std::string, int, double, bool, std::monostate>;

  /**
   * \brief Callback type invoked when a property's value changes.
   *
   * The single argument is the name of the property that changed.
   */
  using PropertyChangedCallback = std::function<void(std::string_view propertyName)>;

  virtual ~PropertySource() = default;

  /**
   * \brief Retrieve a property value by name.
   *
   * \param propertyName The key to look up.
   * \return A Value variant. std::monostate indicates the property is absent.
   */
  virtual Value getProperty(std::string_view propertyName) const = 0;

  /**
   * \brief Whether this source currently has any values available.
   *
   * Implementations may use this to indicate readiness (e.g., after loading
   * from disk or connecting to a remote store).
   */
  virtual bool hasValues() const = 0;

  /**
   * \brief Whether the source is static (values never change at runtime).
   *
   * Static sources will not accept watchers and will never emit change events.
   * \return true if values are immutable for the lifetime of the source.
   */
  virtual bool isStatic() const { return false; }

  /**
   * \brief Convenience helper to check the existence of a property.
   *
   * The default implementation calls getProperty() and checks for std::monostate.
   * \param propertyName The key to test.
   * \return true if a non-monostate value is returned; false otherwise.
   */
  virtual bool containsProperty(std::string_view propertyName) const {
    return !std::holds_alternative<std::monostate>(getProperty(propertyName));
  }

  /**
   * \brief Register a watcher to be notified when properties change.
   *
   * Watchers are only stored when the source is not static. Passing an empty
   * std::function has no effect.
   *
   * \param callback Function to invoke with the property name when a change occurs.
   */
  void addValueWatcher(PropertyChangedCallback callback) {
    if (!isStatic() && callback) {
      _callbacks.emplace_back(callback);
    }
  }

  /**
   * \brief Remove all registered watchers.
   */
  void clearValueWatchers() { _callbacks.clear(); }

private:
  /**
   * \brief Registered callbacks to notify on property changes.
   */
  std::vector<PropertyChangedCallback> _callbacks;
};
}// namespace framework
