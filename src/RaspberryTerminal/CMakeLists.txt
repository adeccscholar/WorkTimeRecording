cmake_minimum_required(VERSION 3.26)

project(App_Terminal)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include (../adecc_tao_settings.cmake)

set(PROJECT_SOURCES RaspTerminal.cpp Sensoring.h)
set(GPIOD_LIBRARIES "/usr/local/lib/libgpiodcxx.so")
			
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES}) 

message(STATUS ${OUTPUT_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

include_directories(${CMAKE_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE CorbaToolsHeader)

target_link_libraries(${PROJECT_NAME} PRIVATE ${GPIOD_LIBRARIES} ProjectTools RaspberryTools)

target_link_libraries(${PROJECT_NAME} PRIVATE Organization_Stubs ${ACE_LIBRARIES} ${TAO_LIBRARIES})


