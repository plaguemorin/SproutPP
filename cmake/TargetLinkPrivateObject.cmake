function(copy_target_build_properties TARGET_NAME LIB)
    get_target_property(INT_INCLUDE_DIRS ${LIB} INTERFACE_INCLUDE_DIRECTORIES)
    if (INT_INCLUDE_DIRS)
        target_include_directories(${TARGET_NAME} SYSTEM PRIVATE ${INT_INCLUDE_DIRS})
    endif ()

    get_target_property(INT_DEFINES ${LIB} INTERFACE_COMPILE_DEFINITIONS)
    if (INT_DEFINES)
        target_compile_definitions(${TARGET_NAME} PRIVATE ${INT_DEFINES})
    endif ()

    # These cannot be queried for "INTERFACE_LIBRARY"
    get_target_property(TARGET_TYPE ${LIB} TYPE)
    if (NOT
            ${TARGET_TYPE}
            STREQUAL
            "INTERFACE_LIBRARY"
    )
        get_target_property(INCLUDE_DIRS ${LIB} INCLUDE_DIRECTORIES)
        if (INCLUDE_DIRS)
            target_include_directories(${TARGET_NAME} SYSTEM PRIVATE ${INCLUDE_DIRS})
        endif ()

        get_target_property(DEFINES ${LIB} COMPILE_DEFINITIONS)
        if (DEFINES)
            target_compile_definitions(${TARGET_NAME} PRIVATE ${DEFINES})
        endif ()
    endif ()

endfunction()

function(target_link_private_object_library TARGET_NAME LIB)
    copy_target_build_properties(${TARGET_NAME} ${LIB})

    #get_target_property(INT_LIBS ${LIB} INTERFACE_LINK_LIBRARIES)
    target_sources(${TARGET_NAME} PRIVATE $<TARGET_OBJECTS:${LIB}>)
endfunction()

function(target_link_private_source_library TARGET_NAME LIB)
    copy_target_build_properties(${TARGET_NAME} ${LIB})

    get_target_property(TARGET_TYPE ${LIB} TYPE)
    if (NOT
            ${TARGET_TYPE}
            STREQUAL
            "INTERFACE_LIBRARY"
    )
        get_target_property(SOURCE_DIR ${LIB} SOURCE_DIR)
        get_target_property(SOURCES ${LIB} SOURCES)
        if (SOURCES)
            foreach (SOURCE_FILE ${SOURCES})
                if (NOT IS_ABSOLUTE ${SOURCE_FILE})
                    target_sources(${TARGET_NAME} PRIVATE ${SOURCE_DIR}/${SOURCE_FILE})
                else ()
                    target_sources(${TARGET_NAME} PRIVATE ${SOURCE_FILE})
                endif ()
            endforeach ()
        endif ()
    endif ()

    add_dependencies(${TARGET_NAME} ${LIB})
endfunction()
