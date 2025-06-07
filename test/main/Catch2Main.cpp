#include <catch2/catch_session.hpp>
#include <spdlog/spdlog.h>

int main(const int argc, char *const argv[]) {
  spdlog::set_level(spdlog::level::trace);
  spdlog::set_pattern("[%H:%M:%S.%e %z] [%5t] [%L] (%s:%#) %v");

  Catch::Session session;

  auto ret = session.applyCommandLine(argc, argv);
  if (ret) {
    return ret;
  }

  return session.run();
}
