cmake_minimum_required(VERSION 3.26)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(../adecc_tao_settings.cmake)

function(generate_idl_group GROUP_NAME)
    set(IDL_FILES ${ARGN})

    set(IDL_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/idl/${GROUP_NAME}")
    file(MAKE_DIRECTORY ${IDL_OUTPUT_DIR})

    set(GENERATED_SRCS)
    set(GENERATED_HDRS)
    set(SERVER_GENERATED_SOURCES)
    set(SERVER_GENERATED_HEADERS)
    set(CLIENT_GENERATED_SOURCES)
    set(CLIENT_GENERATED_HEADERS)

    foreach(IDL_FILE IN LISTS IDL_FILES)
        get_filename_component(IDL_NAME ${IDL_FILE} NAME_WE)
        get_filename_component(IDL_DIR ${IDL_FILE} DIRECTORY)
        message(STATUS "Generating IDL for group '${GROUP_NAME}': ${IDL_FILE}")
        message(STATUS "Directory for include: ${IDL_DIR}")

        set(CPP_C "${IDL_OUTPUT_DIR}/${IDL_NAME}C.cpp")
        set(H_C   "${IDL_OUTPUT_DIR}/${IDL_NAME}C.h")
        set(CPP_S "${IDL_OUTPUT_DIR}/${IDL_NAME}S.cpp")
        set(H_S   "${IDL_OUTPUT_DIR}/${IDL_NAME}S.h")
        set(CPP_I "${IDL_OUTPUT_DIR}/${IDL_NAME}I.cpp")
        set(H_I   "${IDL_OUTPUT_DIR}/${IDL_NAME}I.h")

        # Liste der tatsächlich zu erwartenden Outputs
        set(ALL_OUTPUTS ${CPP_C} ${H_C} ${CPP_S} ${H_S})

        # Versuche optional I-Dateien nur dann einzufügen, wenn sie erzeugt werden
        if(EXISTS ${CPP_I})
            list(APPEND ALL_OUTPUTS ${CPP_I})
        endif()
        if(EXISTS ${H_I})
            list(APPEND ALL_OUTPUTS ${H_I})
        endif()

        add_custom_command(
            OUTPUT ${ALL_OUTPUTS}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${IDL_OUTPUT_DIR}"
            COMMAND tao_idl -I "${IDL_DIR}" -o "${IDL_OUTPUT_DIR}" "${IDL_FILE}"
            DEPENDS "${IDL_FILE}"
            COMMENT "Generating TAO IDL files for ${IDL_NAME} (group ${GROUP_NAME})"
            VERBATIM
        )

        list(APPEND GENERATED_SRCS ${CPP_C} ${CPP_S})
        list(APPEND GENERATED_HDRS ${H_C} ${H_S})

        list(APPEND CLIENT_GENERATED_SOURCES ${CPP_C})
        list(APPEND CLIENT_GENERATED_HEADERS ${H_C})

        list(APPEND SERVER_GENERATED_SOURCES ${CPP_C} ${CPP_S})
        list(APPEND SERVER_GENERATED_HEADERS ${H_C} ${H_S})

        # Falls die Dateien erzeugt werden, hinzufügen
        if(EXISTS ${CPP_I})
            list(APPEND GENERATED_SRCS ${CPP_I})
            list(APPEND SERVER_GENERATED_SOURCES ${CPP_I})
        endif()
        if(EXISTS ${H_I})
            list(APPEND GENERATED_HDRS ${H_I})
            list(APPEND SERVER_GENERATED_HEADERS ${H_I})
        endif()
    endforeach()

    add_custom_target(${GROUP_NAME}_generate_idl_files
        DEPENDS ${GENERATED_SRCS} ${GENERATED_HDRS}
    )

    add_library(${GROUP_NAME}_Stubs STATIC ${CLIENT_GENERATED_SOURCES})
    target_include_directories(${GROUP_NAME}_Stubs PUBLIC ${IDL_OUTPUT_DIR})
    add_dependencies(${GROUP_NAME}_Stubs ${GROUP_NAME}_generate_idl_files)

    add_library(${GROUP_NAME}_Skeletons STATIC ${SERVER_GENERATED_SOURCES})
    target_include_directories(${GROUP_NAME}_Skeletons PUBLIC ${IDL_OUTPUT_DIR})
    add_dependencies(${GROUP_NAME}_Skeletons ${GROUP_NAME}_generate_idl_files)
endfunction()

# Beispiel: eine Gruppe registrieren
generate_idl_group(Organization 
                        "${CMAKE_SOURCE_DIR}/IDL/Basics.idl"
                        "${CMAKE_SOURCE_DIR}/IDL/Organization.idl")

generate_idl_group(Sensors 
                        "${CMAKE_SOURCE_DIR}/IDL/Basics.idl"
                        "${CMAKE_SOURCE_DIR}/IDL/Sensors.idl")
