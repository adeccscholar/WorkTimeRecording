# central cmake settings for using boost in the project

#if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
#   set(BOOST_ROOT "path/to/boost_1_84_0")
#else()
#   set(BOOST_ROOT "path/to/boost_1_84_0")
#endif()

if(WIN32)
   set(Boost_ROOT "D:\\local\\boost_1_87_0")
   set(Boost_DIR "D:\\local\\boost_1_87_0\\lib64-msvc-14.3\\cmake\\Boost-1.87.0")
endif()

# Aktivieren Sie Hot Reload für MSVC-Compiler, sofern unterstützt.
cmake_policy(SET CMP0144 NEW)
cmake_policy(SET CMP0167 NEW)

function(configure_boost_for_target target_name boost_version_param required_components)
    add_definitions(-DWno-dev)

    find_package(Boost  ${boost_version_param} REQUIRED COMPONENTS ${required_components})

    if(Boost_FOUND)
        message(STATUS "[BOOST] Found Boost version ${Boost_VERSION}")
        message(STATUS "[BOOST] Include: ${Boost_INCLUDE_DIRS}")
        message(STATUS "[BOOST] Library: ${Boost_LIBRARY_DIRS}")
        message(STATUS "[BOOST] Libraries: ${Boost_LIBRARIES}")

        # Optional – systemweite Includes und Libverzeichnisse (nicht empfohlen für modern CMake)
        # include_directories(${Boost_INCLUDE_DIRS})
        # link_directories(${Boost_LIBRARY_DIRS})

        # Empfohlen: nur auf Ziel anwenden
        target_include_directories(${target_name} PRIVATE ${Boost_INCLUDE_DIRS})
        target_link_directories(${target_name} PRIVATE ${Boost_LIBRARY_DIRS})
        target_link_libraries(${target_name} PRIVATE ${Boost_LIBRARIES})
    else()
        message(FATAL_ERROR "[BOOST] Boost not found!")
    endif()
endfunction()
