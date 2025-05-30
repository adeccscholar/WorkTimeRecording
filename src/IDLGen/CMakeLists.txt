
include (../adecc_tao_settings.cmake)

set(IDL_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/idl")
file(MAKE_DIRECTORY ${IDL_OUTPUT_DIR})

set(IDL_FILES "${CMAKE_SOURCE_DIR}/IDL/Organization.idl")

# Listen für die generierten Quell- und Header-Dateien
set(GENERATED_SRCS)
set(GENERATED_HDRS)

# Generiere die Dateien mit tao_idl
   foreach(IDL_FILE ${IDL_FILES})
      get_filename_component(IDL_NAME ${IDL_FILE} NAME_WE)
      message(STATUS "IDL:  ${IDL_FILE}")

        # Generierte Dateien für diese IDL
      set(CPP_C "${IDL_OUTPUT_DIR}/${IDL_NAME}C.cpp")
      set(H_C   "${IDL_OUTPUT_DIR}/${IDL_NAME}C.h")
      set(CPP_S "${IDL_OUTPUT_DIR}/${IDL_NAME}S.cpp")
      set(H_S   "${IDL_OUTPUT_DIR}/${IDL_NAME}S.h")
      set(CPP_I "${IDL_OUTPUT_DIR}/${IDL_NAME}I.cpp")
      set(H_I   "${IDL_OUTPUT_DIR}/${IDL_NAME}I.h")

      list(APPEND GENERATED_SRCS ${CPP_C} ${CPP_S} ${CPP_I})
      list(APPEND GENERATED_HDRS ${H_C} ${H_S} ${H_I})

      add_custom_command(
         OUTPUT ${CPP_C} ${H_C} ${CPP_S} ${H_S} ${CPP_I} ${H_I}
         COMMAND tao_idl -o "${IDL_OUTPUT_DIR}" "${IDL_FILE}"
         DEPENDS "${IDL_FILE}"
         COMMENT "Generiere TAO IDL Dateien fuer ${IDL_NAME}"
         VERBATIM
         )

      list(APPEND SERVER_GENERATED_SOURCES ${IDL_OUTPUT_DIR}/${IDL_NAME}S.cpp)
      list(APPEND SERVER_GENERATED_HEADERS ${IDL_OUTPUT_DIR}/${IDL_NAME}S.h)
      list(APPEND SERVER_GENERATED_SOURCES ${IDL_OUTPUT_DIR}/${IDL_NAME}C.cpp)
      list(APPEND SERVER_GENERATED_HEADERS ${IDL_OUTPUT_DIR}/${IDL_NAME}C.h)
      list(APPEND CLIENT_GENERATED_SOURCES ${IDL_OUTPUT_DIR}/${IDL_NAME}C.cpp)
      list(APPEND CLIENT_GENERATED_HEADERS ${IDL_OUTPUT_DIR}/${IDL_NAME}C.h)
   endforeach()

 # Ziel, um alle IDL Dateien zu generieren
  add_custom_target(generate_idl_files ALL DEPENDS ${GENERATED_SRCS} ${GENERATED_HDRS}
     )

# Stubs (Client): Nur C-Dateien
add_library(IDL_Stubs STATIC ${CLIENT_GENERATED_SOURCES})
target_include_directories(IDL_Stubs PUBLIC ${IDL_OUTPUT_DIR})
add_dependencies(IDL_Stubs generate_idl_files)

include_directories("${CMAKE_CURRENT_BINARY_DIR}/idl")

# Skeletons (Server): S- und I-Dateien
add_library(IDL_Skeletons STATIC ${SERVER_GENERATED_SOURCES})
target_include_directories(IDL_Skeletons PUBLIC ${IDL_OUTPUT_DIR})
add_dependencies(IDL_Skeletons generate_idl_files)
