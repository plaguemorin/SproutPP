include(cmake/CPM.cmake)

# Done as a function so that updates to variables like
# CMAKE_CXX_FLAGS don't propagate out to other
# targets
function(sproutpp_setup_dependencies)

    # For each dependency, see if it's
    # already been provided to us by a parent project

    if (NOT TARGET fmtlib::fmtlib)
        CPMAddPackage("gh:fmtlib/fmt#11.1.4")
    endif ()

    if (NOT TARGET spdlog::spdlog)
        CPMAddPackage(
                NAME
                spdlog
                VERSION
                1.15.2
                GITHUB_REPOSITORY
                "gabime/spdlog"
                OPTIONS
                "SPDLOG_FMT_EXTERNAL ON")
    endif ()

    if (NOT TARGET Catch2::Catch2WithMain)
        CPMAddPackage("gh:catchorg/Catch2@3.8.1")
    endif ()

    if (NOT TARGET CLI11::CLI11)
        CPMAddPackage("gh:CLIUtils/CLI11@2.5.0")
    endif ()


    if (NOT TARGET BoostHeaders)
        string(TIMESTAMP timestamp_before_boost "%s")
        message(STATUS "Configuring Boost")

        CPMAddPackage(
                NAME Boost
                VERSION 1.87.0
                URL https://github.com/boostorg/boost/releases/download/boost-1.87.0/boost-1.87.0-cmake.tar.xz
                URL_HASH SHA256=7da75f171837577a52bbf217e17f8ea576c7c246e4594d617bfde7fafd408be5
                DOWNLOAD_ONLY YES
        )
        include(cmake/BoostConfig.cmake)

        string(TIMESTAMP timestamp_after_boost "%s")
        math(EXPR boost_config_duration "${timestamp_after_boost} - ${timestamp_before_boost}")
        message(STATUS "Boost configuration took ${boost_config_duration}s")
    endif ()

endfunction()
