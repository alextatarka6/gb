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