cmake_minimum_required(VERSION 3.26)

project(WeatherClientApp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-DBUILD_WITH_QT)

# include (../adecc_tao_settings.cmake)
include (../adecc_boost_settings.cmake) 

set(PROJECT_SOURCES WeatherClient.cpp )

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES}) 

add_dependencies(${PROJECT_NAME} WeatherAPI)

# configure_boost_for_target(${PROJECT_NAME} 1.87 "system;json")
					

# include(../adecc_qt_settings.cmake)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(${PROJECT_NAME} PRIVATE ProjectTools BoostTools WeatherAPI)

# target_link_libraries(${PROJECT_NAME} PRIVATE ProjectTools adeccDatabase adeccTools)
# target_link_libraries(${PROJECT_NAME} PRIVATE Organization_Skeletons ${ACE_LIBRARIES} ${TAO_LIBRARIES})
