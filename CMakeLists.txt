cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(ZaidFXMod VERSION 0.4.1)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE src)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK. Define GEODE_SDK before configuring the project.")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)
setup_geode_mod(${PROJECT_NAME})

if (ANDROID)
    find_library(MEDIA_NDK_LIBRARY mediandk REQUIRED)
    find_library(LOG_LIBRARY log REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${MEDIA_NDK_LIBRARY} ${LOG_LIBRARY})
endif()
