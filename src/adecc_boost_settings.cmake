# central cmake settings for using boost in the project

#if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
#   set(BOOST_ROOT "path/to/boost_1_84_0")
#else()
#   set(BOOST_ROOT "path/to/boost_1_84_0")
#endif()

if(WIN32)
   set(Boost_ROOT "D:\\local\\boost_1_88_0")
   set(Boost_DIR "D:\\local\\boost_1_88_0\\lib64-msvc-14.3\\cmake\\Boost-1.88.0")
elseif(UNIX)

endif()

# Aktivieren Sie Hot Reload für MSVC-Compiler, sofern unterstützt.
if(WIN32)
cmake_policy(SET CMP0144 NEW)
cmake_policy(SET CMP0167 NEW)
endif()

function(configure_boost_for_target target_name boost_version_param required_components)
   add_definitions(-DWno-dev)

   find_package(Boost  ${boost_version_param} REQUIRED COMPONENTS ${required_components})

   if(Boost_FOUND)
      message(STATUS "[BOOST] Found Boost version ${Boost_VERSION}")
      message(STATUS "[BOOST] Include: ${Boost_INCLUDE_DIRS}")
      message(STATUS "[BOOST] Library: ${Boost_LIBRARY_DIRS}")
      message(STATUS "[BOOST] Libraries: ${Boost_LIBRARIES}")

      get_target_property(type ${target_name} TYPE)
      if(type STREQUAL "INTERFACE_LIBRARY")
         target_include_directories(${target_name} INTERFACE ${Boost_INCLUDE_DIRS})
         target_link_libraries(${target_name} INTERFACE ${Boost_LIBRARIES})
      else()
         target_include_directories(${target_name} PRIVATE ${Boost_INCLUDE_DIRS})
         target_link_libraries(${target_name} PRIVATE ${Boost_LIBRARIES})
      endif()
   else()
      message(FATAL_ERROR "[BOOST] Boost not found!")
   endif()
endfunction()
