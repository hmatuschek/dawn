cmake_minimum_required(VERSION 2.8.8)
project(dawn)

option(ENABLE_FIRMWARE "Enable firmware build." OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)
include(InstallHeadersWithDirectory)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5SerialPort REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Bluetooth REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
if(ENABLE_FIRMWARE)
 find_package(PythonInterp REQUIRED)
endif(ENABLE_FIRMWARE)

ADD_DEFINITIONS(${Qt5Widgets_DEFINITIONS})
INCLUDE_DIRECTORIES(${Qt5Core_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Qt5Declarative_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Qt5Widgets_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Qt5Bluetooth_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Qt5SerialPort_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Qt5Network_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)

LINK_DIRECTORIES(${PROJECT_BINARY_DIR}/src)
set(LIBS ${Qt5Core_LIBRARIES} ${Qt5Widgets_LIBRARIES} ${Qt5SerialPort_LIBRARIES} ${Qt5Bluetooth_LIBRARIES}
  ${Qt5Network_LIBRARIES})

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -ggdb")

# application sources...
add_subdirectory(src)
# firmware sources
if(ENABLE_FIRMWARE)
 add_subdirectory(firmware)
endif(ENABLE_FIRMWARE)

# Source distribution packages:
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME
  "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_SOURCE_IGNORE_FILES
  "/build/;/doc/;/dist/;/.git/;.dat$;.wav$;~$;.qm$;${CPACK_SOURCE_IGNORE_FILES}")

include(CPack)
