add_library(sproutpp_interface INTERFACE
        sproutpp.h
        sproutpp/application_context.h
        sproutpp/application_context_aware.h
        sproutpp/bean_factory.h
        sproutpp/bean_factory_aware.h
        sproutpp/bean_name_aware.h
        sproutpp/property_resolver.h
        sproutpp/property_source.h
)

target_include_directories(sproutpp_interface INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
)

add_library(sproutpp::interface ALIAS sproutpp_interface)
