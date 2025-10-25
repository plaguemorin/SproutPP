add_library(BoostHeaders INTERFACE)

# Recursively gather all `../include` directories in Boost.
file(
    GLOB_RECURSE Boost_DIRS
    LIST_DIRECTORIES true
    "${Boost_SOURCE_DIR}/libs/*"
)
foreach (dir ${Boost_DIRS})
    if (IS_DIRECTORY "${dir}" AND "${dir}" MATCHES "/include$")
        list(APPEND Boost_INCLUDES "${dir}")
    endif ()
endforeach ()

target_include_directories(BoostHeaders SYSTEM BEFORE INTERFACE ${Boost_INCLUDES})
target_compile_definitions(BoostHeaders INTERFACE BOOST_ALL_NO_LIB BOOST_BIND_NO_PLACEHOLDERS)

file(GLOB BoostSystem_SOURCES "${Boost_SOURCE_DIR}/libs/system/src/*")
add_library(BoostSystem OBJECT ${BoostSystem_SOURCES})
target_link_libraries(BoostSystem PUBLIC BoostHeaders)
target_compile_definitions(BoostSystem PUBLIC BOOST_PROGRAM_OPTIONS_NO_LIB BOOST_ASIO_ENABLE_CANCELIO BOOST_SP_USE_STD_ATOMIC)

find_file(HAS_UNISTD_H unistd.h)
if (HAS_UNISTD_H)
    target_compile_definitions(BoostSystem PUBLIC BOOST_HAS_UNISTD_H)
endif ()

file(GLOB BoostFilesystem_SOURCES "${Boost_SOURCE_DIR}/libs/filesystem/src/*")
add_library(BoostFilesystem OBJECT ${BoostFilesystem_SOURCES})
target_link_libraries(BoostFilesystem PUBLIC BoostHeaders BoostSystem)
target_compile_definitions(BoostFilesystem PUBLIC BOOST_FILESYSTEM_NO_CXX20_ATOMIC_REF)

file(GLOB BoostDateTime_SOURCES "${Boost_SOURCE_DIR}/libs/date_time/src/gregorian/greg_month.cpp")
add_library(BoostDateTime OBJECT ${BoostDateTime_SOURCES})
target_link_libraries(BoostDateTime PUBLIC BoostHeaders BoostSystem)
target_compile_definitions(BoostDateTime PUBLIC BOOST_PROGRAM_OPTIONS_NO_LIB)

file(GLOB BoostJson_SOURCES "${Boost_SOURCE_DIR}/libs/json/src/*")
add_library(BoostJson OBJECT ${BoostJson_SOURCES})
target_link_libraries(BoostJson PUBLIC BoostHeaders)

file(GLOB_RECURSE BoostUrl_SOURCES "${Boost_SOURCE_DIR}/libs/url/src/*")
add_library(BoostUrl OBJECT ${BoostUrl_SOURCES})
target_link_libraries(BoostUrl PUBLIC BoostHeaders BoostSystem)

add_dependencies(BoostFilesystem BoostSystem BoostDateTime)
