cmake_minimum_required(VERSION 3.16)

project(insight_server_test VERSION 0.1.0)

set(SRC_HOME_DIR ${HOME_DIR}/src)
set(TEST_HOME_DIR ${HOME_DIR}/src/test)

aux_source_directory(${TEST_HOME_DIR}/utils TEST)
aux_source_directory(${TEST_HOME_DIR}/modules/communication/handler TEST)

list(APPEND DIC_TEST_LIST
        ${TEST}
        ${U_SOCKETS_SRC_LIST}
        ${DIC_UTILS_SRC}
        ${DIC_PROTOCOL_SRC}
        ${DIC_MODULES_SRC_LIST}
        ${DIC_SERVER_SRC}
        ${DIC_CHANNEL_SRC})

add_executable(insight_test ${DIC_TEST_LIST})

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_link_libraries(insight_test
            gtest_main
            uv_a
            UserEnv
            ws2_32
            Iphlpapi
            shlwapi
            sqlite
            -lssp)
else()
    target_link_libraries(insight_test
            gtest_main
            uv_a
            pthread
            dl
            sqlite)
endif()