cmake_minimum_required(VERSION 3.26)

project(WeatherAPI)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-DBUILD_WITH_QT)
add_definitions(-DWEATHERAPI_BUILD_DLL)

include (../adecc_boost_settings.cmake) 

set(PROJECT_SOURCES WeatherData.cpp WeatherData.h
                    WeatherReader.cpp WeatherReader.h)


add_library(WeatherAPI SHARED ${PROJECT_SOURCES} )

target_include_directories(WeatherAPI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(WeatherAPI PROPERTIES 
   RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}       # .dll oder .exe
   ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}       # .lib oder .a
   LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}       # .so oder .dylib
   POSITION_INDEPENDENT_CODE ON
)

# set_target_properties(WeatherAPI PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(WeatherAPI PRIVATE ProjectTools BoostTools)
