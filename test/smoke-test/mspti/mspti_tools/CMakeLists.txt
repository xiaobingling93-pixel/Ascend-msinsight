# libmspti.so
cmake_minimum_required(VERSION 3.14.1)
project(mspti_project)
set(CMAKE_SKIP_RPATH TRUE)
set(TOP_DIR ${CMAKE_SOURCE_DIR}/../)

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

file(GLOB_RECURSE SRC_FILE
    ${CMAKE_SOURCE_DIR}/mspti_lib.cpp
    ${CMAKE_SOURCE_DIR}/common/*.cpp
    ${CMAKE_SOURCE_DIR}/compare/*.cpp
)

add_library(mspti_shared SHARED
    ${SRC_FILE}
)

set_target_properties(mspti_shared
    PROPERTIES
    OUTPUT_NAME mspti
)

if(DEFINED ENV{ASCEND_HOME_PATH})
    set(ASCEND_PATH $ENV{ASCEND_HOME_PATH})
else()
    set(ASCEND_PATH "/usr/local/Ascend/ascend-toolkit/latest")
endif()
set(ASCEND_INCLUDE_BASE_DIR "${ASCEND_PATH}/include")
target_include_directories(mspti_shared PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${ASCEND_INCLUDE_BASE_DIR}/mspti
)

target_compile_options(mspti_shared PRIVATE
    -std=c++17
    -fPIC
    -fstack-protector-all
    -fno-strict-aliasing
    -fno-common
    -fvisibility=hidden
    -fvisibility-inlines-hidden
    -Wfloat-equal
    -Wextra
    -DMSPTI_LIB
    -D_FORTIFY_SOURCE=2
    $<$<NOT:$<STREQUAL:${CMAKE_BUILD_TYPE},Debug>>:-O2>
)

target_link_libraries(mspti_shared PRIVATE
    -Wl,--no-as-needed
    -Wl,--as-needed
    dl
    -lsqlite3
)

install(TARGETS mspti_shared OPTIONAL
    LIBRARY DESTINATION mspti_lib
)

set_target_properties(mspti_shared PROPERTIES
    OUTPUT_NAME "mspti_lib"   # 最终生成 customname.so
    PREFIX ""                  # 去掉 lib 前缀
    SUFFIX ".so"               # 可显式设置后缀
)