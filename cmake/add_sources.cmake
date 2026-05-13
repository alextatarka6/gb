function(add_sources)
    foreach(source ${ARGN})
        if(NOT IS_ABSOLUTE "${source}")
            set(source "${CMAKE_CURRENT_SOURCE_DIR}/${source}")
        endif()

        set_property(
            GLOBAL APPEND PROPERTY
            "ALL_SRC_FILES"
            "${source}"
        )
    endforeach()
endfunction()

# Creates a static library from all sources accumulated in the given directory tree.
function(declare_library TARGET_NAME SOURCES_DIR)
    set_property(GLOBAL PROPERTY ALL_SRC_FILES "")
    add_subdirectory(${SOURCES_DIR})
    get_property(ALL_SOURCES GLOBAL PROPERTY ALL_SRC_FILES)
    add_library(${TARGET_NAME} STATIC ${ALL_SOURCES})
    target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src)
endfunction()

# Creates an executable from all sources accumulated in the given directory tree.
function(declare_executable TARGET_NAME SOURCES_DIR)
    set_property(GLOBAL PROPERTY ALL_SRC_FILES "")
    add_subdirectory(${SOURCES_DIR})
    get_property(ALL_SOURCES GLOBAL PROPERTY ALL_SRC_FILES)
    add_executable(${TARGET_NAME} ${ALL_SOURCES})
endfunction()