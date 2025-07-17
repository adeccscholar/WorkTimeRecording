cmake_minimum_required(VERSION 3.26)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Interface-Library (nur für Client) ---
add_library(CorbaToolsHeader INTERFACE)

# Nur öffentlicher Include-Pfad (mit ausgewählten Headern wie BasicTraits.h)
target_include_directories(CorbaToolsHeader INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> )

target_link_libraries(CorbaToolsHeader INTERFACE Basics_Stubs)

project(CorbaTools)

include (../adecc_tao_settings.cmake)

set(PROJECT_SOURCES Basics_i.cpp Basics_i.h
                    include/BasicTraits.h )

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} PUBLIC CorbaToolsHeader ProjectTools
                                      PRIVATE Basics_Skeletons ${ACE_LIBRARIES} ${TAO_LIBRARIES})


target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES 
   RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}       # .dll oder .exe
   ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}       # .lib oder .a
   LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}       # .so oder .dylib
   POSITION_INDEPENDENT_CODE ON
)

