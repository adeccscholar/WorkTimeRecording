cmake_minimum_required(VERSION 3.26)

project(Weather_Server)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-DBUILD_WITH_QT)

include (../adecc_boost_settings.cmake)

set(PROJECT_SOURCES WeatherServer.cpp)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

configure_boost_for_target(${PROJECT_NAME} 1.87 "system;json")

# include (../adecc_qt_settings.cmake)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

# target_link_libraries(${PROJECT_NAME} PRIVATE ProjectTool adeccDatabase adeccTools)
# TAO im 2. Schritt