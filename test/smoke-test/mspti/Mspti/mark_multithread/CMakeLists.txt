#Copyright (c) Huawei Technologies Co., Ltd. 2019. All rights reserved.

# CMake lowest version requirement
cmake_minimum_required(VERSION 3.14)

# 设置工程名
project(ACLNN_EXAMPLE)

# Compile options
add_compile_options(-std=c++11)

# 设置编译选项
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "./bin")    
set(CMAKE_CXX_FLAGS_DEBUG "-fPIC -O0 -g -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-fPIC -O2 -Wall")

# 设置可执行文件名（如opapi_test），并指定待运行算子文件*.cpp所在目录
add_executable(opapi_test
               test_add.cpp) 

if(DEFINED ASCEND_HOME_PATH)
	set(ASCEND_PATH ${ASCEND_HOME_PATH})
else()
	message("failed to set ASCEND_PATH")
endif()

set(INCLUDE_BASE_DIR "${ASCEND_PATH}/include")
include_directories(
    ${INCLUDE_BASE_DIR}
    ${INCLUDE_BASE_DIR}/acl
    ${INCLUDE_BASE_DIR}/aclnn
)

# 设置链接的库文件路径
target_link_libraries(opapi_test PRIVATE
                      ${ASCEND_PATH}/lib64/libascendcl.so
                      ${ASCEND_PATH}/lib64/libnnopbase.so
		      ${ASCEND_PATH}/lib64/libopapi.so
		      ${ASCEND_PATH}/lib64/libmspti.so)

set_target_properties(opapi_test
    PROPERTIES
    OUTPUT_NAME main
)

# 可执行文件在CMakeLists文件所在目录的bin目录下
install(TARGETS opapi_test DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

