
#pragma once

#include "property_source.h"
#include <functional>

namespace framework {

/**
 * \brief Resolves configuration properties from one or more PropertySource instances.
 *
 * PropertyResolver acts as an indirection layer between the application and various
 * concrete property sources (environment variables, maps, Zookeeper, files, etc.).
 *
 * Implementations are responsible for:
 * - Holding and managing a set of PropertySource objects.
 * - Looking up properties by name across registered sources.
 * - Providing convenience helpers for existence checks and string conversion.
 *
 * Thread-safety is not mandated by this interface. Implementations should document
 * their concurrency guarantees if required by the application.
 */
class PropertyResolver {

protected:
  /**
   * \brief Factory function type used to create PropertySource instances for a given parameter.
   *
   * The parameter meaning (e.g., a connection string, a file path, a namespace, etc.) depends on
   * the concrete PropertySource implementation created by the factory.
   */
  using PropertySourceFactory = std::function<std::unique_ptr<PropertySource>(std::string_view param)>;

  /**
   * \brief Register a factory capable of building a PropertySource for a given symbolic source name.
   *
   * This allows late construction of sources by name, typically used by registerPropertySourceHandler.
   *
   * \param sourceName The symbolic name for the source type (e.g., "env", "map", "zookeeper").
   * \param factory    The factory function that produces a PropertySource when given a parameter.
   */
  virtual void registerPropertySourceFactory(std::string_view sourceName, PropertySourceFactory factory) = 0;

public:
  virtual ~PropertyResolver() = default;

  /**
   * \brief Register a handler (factory) for a PropertySource implementation type.
   *
   * Convenience helper that binds a type T to a sourceName by registering a factory that
   * constructs std::make_unique<T>(param).
   *
   * \tparam T           Concrete type deriving from PropertySource with a constructor T(std::string_view).
   * \param sourceName   The symbolic name used later to refer to this source type.
   *
   * \note The type T must be constructible from a single std::string_view argument.
   */
  template<typename T>
  void registerPropertySourceHandler(std::string_view sourceName) {
    registerPropertySourceFactory(sourceName, [](auto p1) { return std::make_unique<T>(p1); });
  }

  /**
   * \brief Register an already constructed PropertySource instance.
   *
   * Ownership of the provided unique_ptr is transferred to the resolver. Implementations may
   * define ordering or precedence when multiple sources provide the same property.
   *
   * \param source The property source to add.
   */
  virtual void registerPropertySource(std::unique_ptr<PropertySource> &&source) = 0;

  /**
   * \brief Retrieve a property value by name.
   *
   * \param propertyName The property key to look up.
   * \return A PropertySource::Value variant holding one of the supported types
   *         (std::monostate if the property is not present).
   */
  virtual PropertySource::Value getProperty(std::string_view propertyName) = 0;

  /**
   * \brief Check whether a property exists.
   *
   * \param propertyName The property key to test.
   * \return true if a non-monostate value is resolved; false otherwise.
   */
  virtual bool containsProperty(std::string_view propertyName) {
    return !std::holds_alternative<std::monostate>(getProperty(propertyName));
  }

  /**
   * \brief Get a property coerced to string if possible.
   *
   * If the property is present as string/int/double/bool it is converted to a std::string.
   * When the property doesn't exist or cannot be represented as string, defaultValue is returned.
   *
   * \param propertyName  The property key to look up.
   * \param defaultValue  The value to return when the property is missing or not representable.
   * \return The string representation or defaultValue.
   */
  virtual std::string getPropertyAsString(std::string_view propertyName, std::string defaultValue = "") {
    const auto value = getProperty(propertyName);
    if (std::holds_alternative<std::string>(value)) {
      return std::get<std::string>(value);
    }

    if (std::holds_alternative<int>(value)) {
      return std::to_string(std::get<int>(value));
    }

    if (std::holds_alternative<double>(value)) {
      return std::to_string(std::get<double>(value));
    }

    if (std::holds_alternative<bool>(value)) {
      return std::to_string(std::get<bool>(value));
    }
    return defaultValue;
  }

  /**
   * \brief Retrieve a property value as an integer, or return a default value if not present or convertible.
   *
   * This method attempts to fetch the property identified by the specified name and convert it to an integer
   * if possible. Supported conversions include:
   * - Strings: Parsed into integers using `std::stoi`.
   * - Integers: Returned as-is.
   * - Doubles: Cast to integers.
   * - Booleans: Converted to 1 for true, and 0 for false.
   *
   * If the property is not present or cannot be converted, the provided default value is returned.
   *
   * \param propertyName The property key to look up.
   * \param defaultValue The value to return if the property is missing or cannot be converted
   *                     (default is 0).
   * \return The resolved integer value, or defaultValue if the property is absent or unconvertible.
   */
  virtual int getPropertyAsInt(std::string_view propertyName, int defaultValue = {}) {
    const auto value = getProperty(propertyName);
    if (std::holds_alternative<std::string>(value)) {
      return std::stoi(std::get<std::string>(value));
    }

    if (std::holds_alternative<int>(value)) {
      return std::get<int>(value);
    }

    if (std::holds_alternative<double>(value)) {
      return static_cast<int>(std::get<double>(value));
    }

    if (std::holds_alternative<bool>(value)) {
      return std::get<bool>(value) ? 1 : 0;
    }

    return defaultValue;
  }

  /**
   * \brief Retrieve a property value or abort the process if missing.
   *
   * \param propertyName The property key to look up.
   * \return The resolved value.
   * \note This function calls std::abort() if the property is not present. Use with care.
   */
  virtual PropertySource::Value getPropertyRequired(std::string_view propertyName) {
    auto property = getProperty(propertyName);
    if (std::holds_alternative<std::monostate>(property)) {
      std::abort();
    }
    return property;
  }
};

}// namespace framework
