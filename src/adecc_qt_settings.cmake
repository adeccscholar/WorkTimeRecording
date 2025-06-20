# this part can be isolated in a separeted file and later included

if(NOT DEFINED PROJECT_NAME OR PROJECT_NAME STREQUAL "")
    message(FATAL_ERROR "Die Variable PROJECT_NAME ist nicht gesetzt! Bitte definieren bevor diese Datei included wird.")
endif()


if(WIN32)
   message(STATUS "Setting for Windows")
   set(Qt6_DIR "D:/Qt/6.6.2/msvc2019_64/lib/cmake")
   set(Qt_DIR "D:/Qt/6.6.2/msvc2019_64/lib/cmake")
   set(CMAKE_PREFIX_PATH "D:/Qt/6.6.2/msvc2019_64")
   set(QTDIR "D:/Qt/6.6.2/msvc2019_64")
   set(QT_INCLUDE "D:/Qt/6.6.2/msvc2019_64/include")
   set(QT_LIB "D:/Qt/6.6.2/msvc2019_64/lib")
   set(QT_BIN "D:/Qt/6.6.2/msvc2019_64/bin")
   find_package(Qt6 REQUIRED COMPONENTS Core Sql Network)

   add_compile_options(/Zc:__cplusplus) # qt ask for cplusplus version, MS Compiler use MSVER for this usual
else() 
   if("${HOSTNAME}" STREQUAL "Ubuntu1")
      message(STATUS "Setting for Ubuntu1 (Hyper-V)")
      set(Qt_DIR /usr/local/Qt-6.8.2/lib/cmake) 
      set(Qt6_DIR /usr/local/Qt-6.8.2/lib/cmake/Qt6) 
      set(CMAKE_PREFIX_PATH /usr/local/Qt-6.8.2/lib/cmake)
      set(QTDIR /usr/local/Qt-6.8.2)
      set(QT_INCLUDE "/usr/local/Qt-6.8.2/include")
      set(QT_LIB "/usr/local/Qt-6.8.2/lib")
      set(QT_BIN "/usr/local/Qt-6.8.2/bin")
   else()
      message(STATUS "Setting for Raspberries")
      set(Qt6_DIR /usr/local/Qt-6.6.3/lib/cmake)
      set(Qt_DIR /usr/local/Qt-6.6.3/lib/cmake)
      set(CMAKE_PREFIX_PATH /usr/local/Qt-6.6.3)
      set(QTDIR /usr/local/Qt-6.6.3)
      set(QT_INCLUDE "/usr/local/Qt-6.6.3/include")
      set(QT_LIB "/usr/local/Qt-6.6.3/lib")
      set(QT_BIN "/usr/local/Qt-6.6.3/bin")
   endif()
   find_package(Qt6 REQUIRED COMPONENTS Core Sql Network)
endif()

message(STATUS "verwendete Qt- Version: ${QTDIR}")

# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)
# set(CMAKE_AUTOUIC ON)

message(STATUS "Qt include: ${QT_INCLUDE}")
include_directories(${QT_INCLUDE})
include_directories(${QT_INCLUDE}/QtCore ${QT_INCLUDE}/QtSql ${QT_INCLUDE}/QtNetwork)

link_directories("${QT_LIB}")
target_link_directories(${PROJECT_NAME} PUBLIC "${QT_LIB}")
link_directories("${QT_BIN}")

# seems trouble with the private include command 

target_include_directories(${PROJECT_NAME} PUBLIC "${QT_INCLUDE}")
if("${HOSTNAME}" STREQUAL "Ubuntu1")
   target_include_directories(${PROJECT_NAME} PUBLIC "${QT_INCLUDE}/QtCore")
endif()

if(WIN32)
   target_link_libraries(${PROJECT_NAME} PRIVATE Qt::Core Qt::Sql Qt::Network)
else()
   target_link_libraries(${PROJECT_NAME} PRIVATE Qt6Core Qt6Sql Qt6Network)
endif()