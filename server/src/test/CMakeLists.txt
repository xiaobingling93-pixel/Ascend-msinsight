cmake_minimum_required(VERSION 3.16)
project(insight_server_test VERSION 0.1.0)

set(SRC_HOME_DIR ${HOME_DIR}/src)
set(TEST_HOME_DIR ${HOME_DIR}/src/test)

aux_source_directory(${TEST_HOME_DIR} TEST)
aux_source_directory(${TEST_HOME_DIR}/utils TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/communication/handler TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/memory/database TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/memory/handler TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/memory/parser TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/memory/protocol TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/summary TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/summary/protocol TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/summary/handler TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/timeline TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/timeline/core/parser TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/timeline/handler TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/timeline/protocol TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/timeline/database TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/timeline/analyzer TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/timeline/dataEngine TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/timeline/table TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/communication TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/advisor TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/advisor/protocol TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/operator TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/operator/database TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/operator/parser TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/operator/protocol TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/operator/handler TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/common TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/source TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/source/mockUtils TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/global TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/full_db/parserAlloc TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/base/core TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/jupyter/protocol TEST)

aux_source_directory(${TEST_HOME_DIR}/performance PERFORMANCE_TEST)

list(APPEND DIC_TEST_LIST
        ${TEST}
        ${U_SOCKETS_SRC_LIST}
        ${DIC_UTILS_SRC}
        ${DIC_PROTOCOL_SRC}
        ${DIC_MODULES_SRC_LIST}
        ${DIC_SERVER_SRC}
        ${DIC_CHANNEL_SRC})

list(APPEND DIC_PERFORMANCE_TEST_LIST
        ${PERFORMANCE_TEST}
        ${U_SOCKETS_SRC_LIST}
        ${DIC_UTILS_SRC}
        ${DIC_PROTOCOL_SRC}
        ${DIC_MODULES_SRC_LIST}
        ${DIC_SERVER_SRC}
        ${DIC_CHANNEL_SRC})

add_executable(insight_test ${DIC_TEST_LIST} modules/summary/SummaryTest.cpp modules/summary/MockTraceDatabase.h TestSuit.cpp
        FullDbTestSuit.cpp modules/timeline/handler/HandlerTest.cpp modules/timeline/protocol/ProtocolTest.cpp
        modules/timeline/database/DatabaseTest.cpp
        ../utils/NumberSafeUtil.h
        utils/NumberSafeUtilTest.cpp)
add_executable(insight_performance ${DIC_PERFORMANCE_TEST_LIST})
target_link_libraries(insight_test msinsight)
target_link_libraries(insight_performance msinsight)
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_link_libraries(insight_test
            gtest_main gmock_main
            uv_a userenv ws2_32 iphlpapi shlwapi -lssp z
            ${EXECUTABLE_OUTPUT_PATH}/libsqlite.dll)
    target_link_libraries(insight_performance
            gtest_main gmock_main
            uv_a userenv ws2_32 iphlpapi shlwapi -lssp z
            ${EXECUTABLE_OUTPUT_PATH}/libsqlite.dll)
else()
    target_link_libraries(insight_test
            gtest_main gmock_main
            uv_a pthread dl sqlite z)
    target_link_libraries(insight_performance gtest_main gmock_main uv_a pthread dl sqlite z)
endif()

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(insight_performance stdc++fs)
    target_link_libraries(insight_test stdc++fs)
endif()

if (${_PROJECT_TYPE} MATCHES "fuzz")
    add_subdirectory(fuzz)
endif ()