cmake_minimum_required(VERSION 3.26)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(../adecc_tao_settings.cmake)

function(generate_idl_group GROUP_NAME)
   set(options)
   set(oneValueArgs)
   set(multiValueArgs IDL_FILES PRECOMPILED_DEPENDENCIES)
   cmake_parse_arguments(GIG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

   set(IDL_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/idl/${GROUP_NAME}")
   file(MAKE_DIRECTORY ${IDL_OUTPUT_DIR})

   set(GENERATED_SRCS)
   set(GENERATED_HDRS)
   set(SERVER_GENERATED_SOURCES)
   set(SERVER_GENERATED_HEADERS)
   set(CLIENT_GENERATED_SOURCES)
   set(CLIENT_GENERATED_HEADERS)

   foreach(IDL_FILE IN LISTS GIG_IDL_FILES)
      get_filename_component(IDL_NAME ${IDL_FILE} NAME_WE)
      get_filename_component(IDL_DIR ${IDL_FILE} DIRECTORY)

      message(STATUS "Generating IDL for group '${GROUP_NAME}': ${IDL_FILE}")
      message(STATUS "IDL include directory: ${IDL_DIR}")

      set(CPP_C "${IDL_OUTPUT_DIR}/${IDL_NAME}C.cpp")
      set(H_C   "${IDL_OUTPUT_DIR}/${IDL_NAME}C.h")
      set(CPP_S "${IDL_OUTPUT_DIR}/${IDL_NAME}S.cpp")
      set(H_S   "${IDL_OUTPUT_DIR}/${IDL_NAME}S.h")
      set(CPP_I "${IDL_OUTPUT_DIR}/${IDL_NAME}I.cpp")
      set(H_I   "${IDL_OUTPUT_DIR}/${IDL_NAME}I.h")

      set(ALL_OUTPUTS ${CPP_C} ${H_C} ${CPP_S} ${H_S})

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

   # Verlinkung & Header-Propagation für Precompiled Dependencies
   foreach(DEP ${GIG_PRECOMPILED_DEPENDENCIES})
      # Header-Include auf das ORIGINAL-Ziel (z.B. idl/Basics)
      target_include_directories(${GROUP_NAME}_Stubs PUBLIC
         ${CMAKE_CURRENT_BINARY_DIR}/idl/${DEP}
      )
      target_include_directories(${GROUP_NAME}_Skeletons PUBLIC
         ${CMAKE_CURRENT_BINARY_DIR}/idl/${DEP}
      )

      # Verlinkung gegen Stub/Skeleton
      target_link_libraries(${GROUP_NAME}_Stubs PUBLIC ${DEP}_Stubs)
      target_link_libraries(${GROUP_NAME}_Skeletons PUBLIC ${DEP}_Skeletons)
   endforeach()

   # Nachträgliches Löschen von doppelt generierten .h/.cpp/.inl
   foreach(DEP ${GIG_PRECOMPILED_DEPENDENCIES})
      file(GLOB_RECURSE DUP_FILES
         "${IDL_OUTPUT_DIR}/${DEP}C.*"
         "${IDL_OUTPUT_DIR}/${DEP}S.*"
         "${IDL_OUTPUT_DIR}/${DEP}*.inl"
      )

      if(DUP_FILES)
         message(STATUS "Cleaning up duplicate IDL-generated files for '${DEP}' in group '${GROUP_NAME}': ${DUP_FILES}")
         file(REMOVE ${DUP_FILES})
      endif()
   endforeach()
endfunction()

# --------------------------------------
# IDL-Gruppen registrieren
# --------------------------------------

# Basics wird zentral erzeugt
generate_idl_group(Basics
   IDL_FILES "${CMAKE_SOURCE_DIR}/IDL/Basics.idl"
)

# Organization nutzt Basics
generate_idl_group(Organization
   IDL_FILES "${CMAKE_SOURCE_DIR}/IDL/Organization.idl"
   PRECOMPILED_DEPENDENCIES Basics
)

# Sensors ebenfalls
generate_idl_group(Weather
   IDL_FILES "${CMAKE_SOURCE_DIR}/IDL/Weather.idl"
   PRECOMPILED_DEPENDENCIES Basics
)

# Sensors ebenfalls
generate_idl_group(Sensors
   IDL_FILES "${CMAKE_SOURCE_DIR}/IDL/Sensors.idl"
   PRECOMPILED_DEPENDENCIES Basics
)
