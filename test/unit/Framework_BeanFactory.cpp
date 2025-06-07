#include "catch2/catch_session.hpp"
#include "sproutpp/application_context.h"
#include <catch2/catch_test_macros.hpp>

namespace {
auto createApplicationContext(const char* argv0) {
  char *argv[] = {const_cast<char *>(argv0), nullptr};
  auto application_context = framework::ApplicationContext::Create(1, argv);
  application_context->initialize();
  return application_context;
}
}// namespace

TEST_CASE("Can create a default bean factory") {
  const auto ac = createApplicationContext(__FUNCTION__);
  REQUIRE(ac != nullptr);
}

TEST_CASE("Can allocate a bean") {
  struct Tst {};

  const auto ac = createApplicationContext(__FUNCTION__);
  auto test = ac->createSingleton<Tst>();

  REQUIRE(test != nullptr);
}


TEST_CASE("Bean Factory Aware is called") {
  struct Tst : framework::BeanFactoryAware {
    bool called = false;
    void setBeanFactory(framework::BeanFactory &factory) override {
      (void) factory;
      called = true;
    }
  };

  const auto ac = createApplicationContext(__FUNCTION__);
  auto test = ac->createSingleton<Tst>();

  REQUIRE(test != nullptr);
  REQUIRE(test->called == true);
}

TEST_CASE("Can find bean") {
  struct Tst {};
  const auto ac = createApplicationContext(__FUNCTION__);
  const auto test1 = ac->createSingleton<Tst>();
  const auto test2 = ac->getFirstBeanTyped<Tst>();

  REQUIRE(test1 == test2);
}

TEST_CASE("Can name beans") {
  struct Tst {};
  const auto ac = createApplicationContext(__FUNCTION__);

  {
    auto tstRaw = std::make_unique<Tst>();
    ac->registerExistingBean(std::move(tstRaw), "test");
  }

  REQUIRE(ac->getBeanTyped<Tst>("test") != nullptr);
  REQUIRE(ac->getBeanTyped<Tst>("test2") == nullptr);
}
