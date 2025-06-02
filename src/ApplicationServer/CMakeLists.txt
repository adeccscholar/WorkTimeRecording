cmake_minimum_required(VERSION 3.26)

project(App_Server)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include (../adecc_tao_settings.cmake)

set(PROJECT_SOURCES AppServer.cpp
                    EmployeeData.h
                    Employee_i.cpp Employee_i.h
                    Company_i.cpp Company_i.h)
					
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES}) 

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(${PROJECT_NAME} PRIVATE ProjectTools)
target_link_libraries(${PROJECT_NAME} PRIVATE Organization_Skeletons ${ACE_LIBRARIES} ${TAO_LIBRARIES})

# target_link_libraries(${PROJECT_NAME} PRIVATE Organization_Skeletons ${ACE_LIBRARIES} ${TAO_LIBRARIES})


