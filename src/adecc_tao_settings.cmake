# attention, befor this included IDL_FILES must be set in CmakeList.txt

if(NOT DEFINED PROJECT_NAME OR PROJECT_NAME STREQUAL "")
    message(FATAL_ERROR "Die Variable PROJECT_NAME ist nicht gesetzt! Bitte definieren bevor diese Datei included wird.")
endif()

message(STATUS "processing TAO setting file for ${PROJECT_NAME}")

if(WIN32)
   message(STATUS "Build for Windows")
   set(ACE_INCLUDE_DIR "D:\\Projekte\\Build\\ACE_wrappers")
   set(TAO_INCLUDE_DIR "D:\\Projekte\\Build\\ACE_wrappers\\TAO" "D:\\Projekte\\Build\\ACE_wrappers\\TAO\\orbsvcs")
   set(ACE_LIB_DIR "D:\\Projekte\\Build\\ACE_wrappers\\lib")
   set(TAO_LIB_DIR "D:\\Projekte\\Build\\ACE_wrappers\\lib")
   set(ACE_LIBRARIES ACE)
   set(TAO_LIBRARIES TAO TAO_AnyTypeCode TAO_PortableServer TAO_CosNaming)
elseif(UNIX)
   if("${HOSTNAME}" STREQUAL "Ubuntu1")
      set(ACE_INCLUDE_DIR "/usr/local/include")
      set(TAO_INCLUDE_DIR "/usr/local/include")
   elseif("${HOSTNAME}" STREQUAL "raspberrypi")
      message(STATUS "Build for RaspberryPi (at home)")
      set(ACE_INCLUDE_DIR "/home/vhillmann/Downloads/ACE_Wrappers")
      set(TAO_INCLUDE_DIR "/home/vhillmann/Downloads/ACE_Wrappers/TAO" "/home/vhillmann/Downloads/ACE_Wrappers/TAO/orbsvcs")
   elseif("${HOSTNAME}" STREQUAL "raspberrypi2")
      message(STATUS "Build for RaspberryPi2 (at office)")
      set(ACE_INCLUDE_DIR "/home/vhillmann/build/ACE_Wrappers")
      set(TAO_INCLUDE_DIR "/home/vhillmann/build/ACE_Wrappers/TAO" "/home/vhillmann/build/ACE_Wrappers/TAO/orbsvcs")
   endif()
   set(ACE_LIB_DIR "/usr/local/lib/")
   set(TAO_LIB_DIR "/usr/local/lib/")
   set(ACE_LIBRARIES ACE)
   set(TAO_LIBRARIES TAO TAO_AnyTypeCode TAO_PortableServer TAO_CosNaming)
endif()

add_definitions(-D_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS)

include_directories(${ACE_INCLUDE_DIR} ${TAO_INCLUDE_DIR})
link_directories(${ACE_LIB_DIR} ${TAO_LIB_DIR})


