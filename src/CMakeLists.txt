cmake_minimum_required(VERSION 3.26)

project(Time_Tracking)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)



include (adecc_cmake_tools_settings.cmake)

# define output directory and set it for same points
set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/out/${PLATFORM}/${CMAKE_BUILD_TYPE}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})

# Add the interface library for Tools
add_library(ProjectTools INTERFACE)

# Register the include directory with this interface library
target_include_directories(ProjectTools INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Tools)

# Add subprojects
add_subdirectory(IDLGen)
add_subdirectory(ApplicationServer)
add_subdirectory(Client)
if(PLATFORM STREQUAL "Raspberry")
   add_subdirectory(RaspberryTerminal)
endif()


