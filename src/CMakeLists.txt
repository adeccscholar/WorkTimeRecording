cmake_minimum_required(VERSION 3.26)

project(Time_Tracking)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(ADECC_TOOLS_REPO_PATH ${CMAKE_SOURCE_DIR}/external/adecc_Tools)
# set(ADECC_SCHOLAR_REPO_PATH ${CMAKE_SOURCE_DIR}/external/adecc_Scholar)
set(ADECC_DATABASE_REPO_PATH ${CMAKE_SOURCE_DIR}/external/adecc_Database)

include (adecc_cmake_tools_settings.cmake)

# define output directory and set it for same points
set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/out/${PLATFORM}/${CMAKE_BUILD_TYPE}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})

clone_repository(ADECC_TOOLS_REPO_PATH "adecc Tools" "https://github.com/adeccscholar/adecc_Scholar_Tools" FALSE)
clone_repository(ADECC_DATABASE_REPO_PATH "adecc Database" "https://github.com/adeccscholar/adecc_Scholar_Database" FALSE)

add_include_directory_if_exists(ADECC_TOOLS_REPO_PATH adeccTools)
add_include_directory_if_exists(ADECC_DATABASE_REPO_PATH adeccDatabase)

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


