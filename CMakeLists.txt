cmake_minimum_required(VERSION 3.5)

project(Digital-Twin-Server VERSION 0.1 LANGUAGES CXX)



set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(HIK_SDK_DIR /home/climatex/Desktop/Lib-Zone/hik_sdk)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
	set(HIK_SDK_DIR G:/Development)
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")



set(STANDARD_IMAGE_SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/templates)
set(STANDARD_IMAGE_DST_PATH ${CMAKE_CURRENT_BINARY_DIR}/templates)

IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
	set (OpenCV_DIR D:/opencv/build/x64/vc15/lib)
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Windows")


execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory  "${STANDARD_IMAGE_SRC_PATH}" "${STANDARD_IMAGE_DST_PATH}")

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets Network REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Network REQUIRED)
find_package(OpenCV REQUIRED)

file(
    GLOB_RECURSE
    SOURCE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*
)

file(
    GLOB_RECURSE
    HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*
)

file(
    GLOB_RECURSE
    UI
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/*
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
	    ${HIK_SDK_DIR}/Includes/linux
    ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
        ${HIK_SDK_DIR}/Includes
    ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    
)

set(PROJECT_SOURCES

    ${SOURCE}
    ${HEADER}
    ${UI}

)

add_executable(
    Digital-Twin-Server
    ${PROJECT_SOURCES}
)

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
	target_link_directories(Digital-Twin-Server PRIVATE ${HIK_SDK_DIR}/lib/64)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
	target_link_directories(Digital-Twin-Server PRIVATE ${HIK_SDK_DIR}/Libraries/win64)
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
	target_link_libraries(Digital-Twin-Server PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network ${OpenCV_LIBS} MvCameraControl)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
	target_link_libraries(Digital-Twin-Server PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network ${OpenCV_LIBS} MvCameraControl.lib)
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")


set_target_properties(Digital-Twin-Server PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Digital-Twin-Server)
endif()