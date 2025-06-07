
add_library(sproutpp_framework STATIC
        bean_utils.cpp
        compositing_property_resolver.cpp
        compositing_property_resolver.h
        context_factory.cpp
        default_application_context.cpp
        default_application_context.h
        default_bean_factory_impl.cpp
        default_bean_factory_impl.h
)

add_subdirectory(property_sources)
target_link_libraries(sproutpp_framework PUBLIC sproutpp_interface)
add_library(sproutpp::framework ALIAS sproutpp_framework)

target_link_private_source_library(sproutpp_framework BoostHeaders)
target_link_private_source_library(sproutpp_framework spdlog::spdlog)
target_link_private_source_library(sproutpp_framework fmt::fmt)
target_link_private_source_library(sproutpp_framework CLI11::CLI11)
