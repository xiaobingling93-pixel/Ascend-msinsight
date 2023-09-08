cmake_minimum_required(VERSION 3.16)

project(profiler_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

set(HOME_DIR ${PROJECT_SOURCE_DIR})
message(STATUS "HOME_DIR: ${HOME_DIR}")

message(STATUS "System name = ${CMAKE_SYSTEM_NAME}")
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(EXECUTABLE_OUTPUT_PATH ${HOME_DIR}/output/win_mingw64/bin)
    set(LIBRARY_OUTPUT_PATH ${HOME_DIR}/output/win_mingw64/lib)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(EXECUTABLE_OUTPUT_PATH ${HOME_DIR}/output/linux_x86_64/bin)
    set(LIBRARY_OUTPUT_PATH ${HOME_DIR}/output/linux_x86_64/lib)
elseif(CMAKE_SYSTEM_NAME MATCHES "Emscripten")
    set(EXECUTABLE_OUTPUT_PATH ${HOME_DIR}/output/emscripten)
    set(LIBRARY_OUTPUT_PATH ${HOME_DIR}/output/emscripten)
else()
    set(EXECUTABLE_OUTPUT_PATH ${HOME_DIR}/output/darwin/bin)
    set(LIBRARY_OUTPUT_PATH ${HOME_DIR}/output/darwin/lib)
endif()

# windows add link option: -static
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    if ((NOT CMAKE_BUILD_TYPE MATCHES "Debug") AND (NOT CMAKE_BUILD_TYPE MATCHES "PROFILE"))
        message(STATUS "Build type = ${CMAKE_BUILD_TYPE}, static = enable.")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--nxcompat")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--dynamicbase")
        add_link_options(-static)
    endif()
endif()

message(STATUS "Project Type = ${_PROJECT_TYPE}, Project SubType = ${_PROJECT_SUBTYPE},CMAKE BUILD TYPE = ${CMAKE_BUILD_TYPE}")

include(cmake/third_party.cmake)
add_subdirectory(src)
