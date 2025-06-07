add_executable(
        SproutPP_UnitTests
        Framework_BeanFactory.cpp
        Framework_PropertiesSource.cpp
        Framework_EnvironmentPropertySource.cpp
)

target_link_libraries(
        SproutPP_UnitTests
        PRIVATE
        sproutpp::sproutpp_warnings
        sproutpp::sproutpp_options
        sproutpp::interface
        sproutpp::framework
        Catch2Main
)

target_include_directories(SproutPP_UnitTests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_private_source_library(SproutPP_UnitTests BoostHeaders)
catch_discover_tests(SproutPP_UnitTests)

if (WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(
            TARGET SproutPP_UnitTests
            PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:tests> $<TARGET_FILE_DIR:tests>
            COMMAND_EXPAND_LISTS)
endif ()

# automatically discover tests that are defined in catch based test files you can modify the unittests. Set TEST_PREFIX
# to whatever you want, or use different for different binaries
catch_discover_tests(
        SproutPP_UnitTests
        TEST_PREFIX
        "unittests."
        REPORTER
        XML
        OUTPUT_DIR
        .
        OUTPUT_PREFIX
        "unittests."
        OUTPUT_SUFFIX
        .xml)
