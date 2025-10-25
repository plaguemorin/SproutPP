add_library(Catch2Main STATIC Catch2Main.cpp)

target_include_directories(Catch2Main PRIVATE .. "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(
        Catch2Main PRIVATE
        sproutpp::sproutpp_warnings
        sproutpp::sproutpp_options
        spdlog::spdlog_header_only
        fmt::fmt-header-only
)

target_link_system_libraries(Catch2Main PUBLIC Catch2::Catch2)


