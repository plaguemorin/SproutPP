include(cmake/SystemLink.cmake)
include(cmake/LibFuzzer.cmake)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)


include(CheckCXXSourceCompiles)


macro(sproutpp_supports_sanitizers)
    if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)

        message(STATUS "Sanity checking UndefinedBehaviorSanitizer, it should be supported on this platform")
        set(TEST_PROGRAM "int main() { return 0; }")

        # Check if UndefinedBehaviorSanitizer works at link time
        set(CMAKE_REQUIRED_FLAGS "-fsanitize=undefined")
        set(CMAKE_REQUIRED_LINK_OPTIONS "-fsanitize=undefined")
        check_cxx_source_compiles("${TEST_PROGRAM}" HAS_UBSAN_LINK_SUPPORT)

        if(HAS_UBSAN_LINK_SUPPORT)
            message(STATUS "UndefinedBehaviorSanitizer is supported at both compile and link time.")
            set(SUPPORTS_UBSAN ON)
        else()
            message(WARNING "UndefinedBehaviorSanitizer is NOT supported at link time.")
            set(SUPPORTS_UBSAN OFF)
        endif()
    else()
        set(SUPPORTS_UBSAN OFF)
    endif()

    if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
        set(SUPPORTS_ASAN OFF)
    else()
        if (NOT WIN32)
            message(STATUS "Sanity checking AddressSanitizer, it should be supported on this platform")
            set(TEST_PROGRAM "int main() { return 0; }")

            # Check if AddressSanitizer works at link time
            set(CMAKE_REQUIRED_FLAGS "-fsanitize=address")
            set(CMAKE_REQUIRED_LINK_OPTIONS "-fsanitize=address")
            check_cxx_source_compiles("${TEST_PROGRAM}" HAS_ASAN_LINK_SUPPORT)

            if(HAS_ASAN_LINK_SUPPORT)
                message(STATUS "AddressSanitizer is supported at both compile and link time.")
                set(SUPPORTS_ASAN ON)
            else()
                message(WARNING "AddressSanitizer is NOT supported at link time.")
                set(SUPPORTS_ASAN OFF)
            endif()
        else()
            set(SUPPORTS_ASAN ON)
        endif()
    endif()
endmacro()

macro(sproutpp_setup_options)
    option(sproutpp_ENABLE_HARDENING "Enable hardening" ON)
    option(sproutpp_ENABLE_COVERAGE "Enable coverage reporting" OFF)
    cmake_dependent_option(
            sproutpp_ENABLE_GLOBAL_HARDENING
            "Attempt to push hardening options to built dependencies"
            ON
            sproutpp_ENABLE_HARDENING
            OFF)

    sproutpp_supports_sanitizers()

    if(NOT PROJECT_IS_TOP_LEVEL OR sproutpp_PACKAGING_MAINTAINER_MODE)
        option(sproutpp_ENABLE_IPO "Enable IPO/LTO" OFF)
        option(sproutpp_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
        option(sproutpp_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
        option(sproutpp_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
        option(sproutpp_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
        option(sproutpp_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
        option(sproutpp_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
        option(sproutpp_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
        option(sproutpp_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
        option(sproutpp_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
        option(sproutpp_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
        option(sproutpp_ENABLE_PCH "Enable precompiled headers" OFF)
        option(sproutpp_ENABLE_CACHE "Enable ccache" OFF)
    else()
        option(sproutpp_ENABLE_IPO "Enable IPO/LTO" OFF)
        option(sproutpp_WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
        option(sproutpp_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
        option(sproutpp_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF) #${SUPPORTS_ASAN})
        option(sproutpp_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
        option(sproutpp_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
        option(sproutpp_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
        option(sproutpp_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
        option(sproutpp_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
        option(sproutpp_ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
        option(sproutpp_ENABLE_CPPCHECK "Enable cpp-check analysis" ON)
        option(sproutpp_ENABLE_PCH "Enable precompiled headers" OFF)
        option(sproutpp_ENABLE_CACHE "Enable ccache" ON)
    endif()

    if(NOT PROJECT_IS_TOP_LEVEL)
        mark_as_advanced(
                sproutpp_ENABLE_IPO
                sproutpp_WARNINGS_AS_ERRORS
                sproutpp_ENABLE_USER_LINKER
                sproutpp_ENABLE_SANITIZER_ADDRESS
                sproutpp_ENABLE_SANITIZER_LEAK
                sproutpp_ENABLE_SANITIZER_UNDEFINED
                sproutpp_ENABLE_SANITIZER_THREAD
                sproutpp_ENABLE_SANITIZER_MEMORY
                sproutpp_ENABLE_UNITY_BUILD
                sproutpp_ENABLE_CLANG_TIDY
                sproutpp_ENABLE_CPPCHECK
                sproutpp_ENABLE_COVERAGE
                sproutpp_ENABLE_PCH
                sproutpp_ENABLE_CACHE)
    endif()

    sproutpp_check_libfuzzer_support(LIBFUZZER_SUPPORTED)
    if(LIBFUZZER_SUPPORTED AND (sproutpp_ENABLE_SANITIZER_ADDRESS OR sproutpp_ENABLE_SANITIZER_THREAD OR sproutpp_ENABLE_SANITIZER_UNDEFINED))
        set(DEFAULT_FUZZER ON)
    else()
        set(DEFAULT_FUZZER OFF)
    endif()

    option(sproutpp_BUILD_FUZZ_TESTS "Enable fuzz testing executable" ${DEFAULT_FUZZER})

endmacro()

macro(sproutpp_global_options)
    if(sproutpp_ENABLE_IPO)
        include(cmake/InterproceduralOptimization.cmake)
        sproutpp_enable_ipo()
    endif()

    sproutpp_supports_sanitizers()

    if(sproutpp_ENABLE_HARDENING AND sproutpp_ENABLE_GLOBAL_HARDENING)
        include(cmake/Hardening.cmake)
        if(NOT SUPPORTS_UBSAN
                OR sproutpp_ENABLE_SANITIZER_UNDEFINED
                OR sproutpp_ENABLE_SANITIZER_ADDRESS
                OR sproutpp_ENABLE_SANITIZER_THREAD
                OR sproutpp_ENABLE_SANITIZER_LEAK)
            set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
        else()
            set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
        endif()
        message("${sproutpp_ENABLE_HARDENING} ${ENABLE_UBSAN_MINIMAL_RUNTIME} ${sproutpp_ENABLE_SANITIZER_UNDEFINED}")
        sproutpp_enable_hardening(sproutpp_options ON ${ENABLE_UBSAN_MINIMAL_RUNTIME})
    endif()
endmacro()

macro(sproutpp_local_options)
    if(PROJECT_IS_TOP_LEVEL)
        include(cmake/StandardProjectSettings.cmake)
    endif()

    add_library(sproutpp_warnings INTERFACE)
    add_library(sproutpp_options INTERFACE)

    include(cmake/CompilerWarnings.cmake)
    sproutpp_set_project_warnings(
            sproutpp_warnings
            ${sproutpp_WARNINGS_AS_ERRORS}
            ""
            ""
            ""
            "")

    if(sproutpp_ENABLE_USER_LINKER)
        include(cmake/Linker.cmake)
        sproutpp_configure_linker(sproutpp_options)
    endif()

    include(cmake/Sanitizers.cmake)
    sproutpp_enable_sanitizers(
            sproutpp_options
            ${sproutpp_ENABLE_SANITIZER_ADDRESS}
            ${sproutpp_ENABLE_SANITIZER_LEAK}
            ${sproutpp_ENABLE_SANITIZER_UNDEFINED}
            ${sproutpp_ENABLE_SANITIZER_THREAD}
            ${sproutpp_ENABLE_SANITIZER_MEMORY})

    set_target_properties(sproutpp_options PROPERTIES UNITY_BUILD ${sproutpp_ENABLE_UNITY_BUILD})

    if(sproutpp_ENABLE_PCH)
        target_precompile_headers(
                sproutpp_options
                INTERFACE
                <vector>
                <string>
                <utility>)
    endif()

    if(sproutpp_ENABLE_CACHE)
        include(cmake/Cache.cmake)
        sproutpp_enable_cache()
    endif()

    include(cmake/StaticAnalyzers.cmake)
    if(sproutpp_ENABLE_CLANG_TIDY)
        sproutpp_enable_clang_tidy(sproutpp_options ${sproutpp_WARNINGS_AS_ERRORS})
    endif()

    if(sproutpp_ENABLE_CPPCHECK)
        sproutpp_enable_cppcheck(${sproutpp_WARNINGS_AS_ERRORS} "" # override cppcheck options
        )
    endif()

    if(sproutpp_ENABLE_COVERAGE)
        include(cmake/Tests.cmake)
        sproutpp_enable_coverage(sproutpp_options)
    endif()

    if(sproutpp_WARNINGS_AS_ERRORS)
        check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
        if(LINKER_FATAL_WARNINGS)
            # This is not working consistently, so disabling for now
            # target_link_options(sproutpp_options INTERFACE -Wl,--fatal-warnings)
        endif()
    endif()

    if(sproutpp_ENABLE_HARDENING AND NOT sproutpp_ENABLE_GLOBAL_HARDENING)
        include(cmake/Hardening.cmake)
        if(NOT SUPPORTS_UBSAN
                OR sproutpp_ENABLE_SANITIZER_UNDEFINED
                OR sproutpp_ENABLE_SANITIZER_ADDRESS
                OR sproutpp_ENABLE_SANITIZER_THREAD
                OR sproutpp_ENABLE_SANITIZER_LEAK)
            set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
        else()
            set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
        endif()
        sproutpp_enable_hardening(sproutpp_options OFF ${ENABLE_UBSAN_MINIMAL_RUNTIME})
    endif()

endmacro()
