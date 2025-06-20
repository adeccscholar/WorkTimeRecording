
if(NOT DEFINED PROJECT_NAME OR PROJECT_NAME STREQUAL "")
    message(FATAL_ERROR "Die Variable PROJECT_NAME ist nicht gesetzt! Bitte definieren bevor diese Datei included wird.")
endif()

# define the plattform about system name and make plattform spezific settings
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PLATFORM "Windows")
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${OUTPUT_DIR}")
    add_definitions(-D_WIN32_WINNT=0x0A00) # Windows 10 / Server 2019, for Windows 7 is 0x0601
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
   if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
      set(PLATFORM "Raspberry")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=cortex-a53 -O2")
      # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=cortex-a53 -O2")
   else()
      set(PLATFORM "Linux")
      set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -ltbb -DNDEBUG")
      set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -DDEBUG")
      # find_package(TBB REQUIRED)
      # target_link_libraries(MyApp PRIVATE TBB::tbb)
   endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(PLATFORM "macOS")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(PLATFORM "Android")
endif()



macro(clone_repository REPRO_PATH REPRO_NAME REPRO_URL UPDATE_IF_EXISTS)
   if(DEFINED ${REPRO_PATH})
      if(NOT EXISTS ${${REPRO_PATH}})
         message("Cloning ${REPRO_NAME} repository ...")
         execute_process(
            COMMAND git clone ${REPRO_URL} ${${REPRO_PATH}} 
            RESULT_VARIABLE GIT_CLONE_RESULT 
         )
         if(NOT ${GIT_CLONE_RESULT} EQUAL 0)
            message(FATAL_ERROR "Failed to clone ${REPRO_NAME} repository. Aborting.")
         endif()
      else()
         if(${UPDATE_IF_EXISTS})
            message("Repository ${REPRO_NAME} already exists. Updating ...")
            execute_process(
               COMMAND git pull
               WORKING_DIRECTORY ${${REPRO_PATH}}
               RESULT_VARIABLE GIT_PULL_RESULT
            )
            if(NOT ${GIT_PULL_RESULT} EQUAL 0)
               message(WARNING "Failed to update ${REPRO_NAME} repository.")
            endif()
         else()
            message("Repository ${REPRO_NAME} already exists. Skipping update.")
         endif()
      endif()
   else()
      message(WARNING "${REPRO_PATH} is not defined. Skipping repository cloning.")
   endif()
endmacro()


macro(add_include_directory_if_exists INCLUDE_PATH LIBRARY_NAME)
   if(DEFINED ${INCLUDE_PATH})
      if(EXISTS ${${INCLUDE_PATH}})
         add_library(${LIBRARY_NAME} INTERFACE)
         target_include_directories(${LIBRARY_NAME} INTERFACE ${${INCLUDE_PATH}})
      else()
         message(WARNING "Include directory ${${INCLUDE_PATH}} does not exist.")
      endif()
   else()
      message(WARNING "${INCLUDE_PATH} is not defined.")
   endif()
endmacro()



# extract the hostname for individual settings
execute_process(COMMAND hostname OUTPUT_VARIABLE HOSTNAME OUTPUT_STRIP_TRAILING_WHITESPACE)
string(STRIP "${HOSTNAME}" TRIMMED_HOSTNAME)
message(STATUS "Hostname: ${HOSTNAME}")

message(STATUS "System: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Processor: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Platform: ${CMAKE_HOST_SYSTEM_NAME}")

