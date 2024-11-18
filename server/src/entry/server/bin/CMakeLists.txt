project(profiler_server)

aux_source_directory(./ SERVER_BIN_SRC)
list(APPEND SERVER_SRC_LIST
        ${SERVER_BIN_SRC}
        ${U_SOCKETS_SRC_LIST}
        ${DIC_UTILS_SRC}
        ${DIC_PROTOCOL_SRC}
        ${DIC_MODULES_SRC_LIST}
        ${DIC_SERVER_SRC}
        ${DIC_CHANNEL_SRC})

add_executable(${PROJECT_NAME} ${SERVER_SRC_LIST})
# 添加预编译头文件到目标
target_precompile_headers(${PROJECT_NAME} PRIVATE ${SRC_HOME_DIR}/entry/server/bin/include/pch.h)
# U_SOCKETS_SRC_LIST无法使用预编译头文件，故排除
set_source_files_properties(${U_SOCKETS_SRC_LIST} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_link_libraries(${PROJECT_NAME}
            ${EXECUTABLE_OUTPUT_PATH}/libsqlite.dll)
    target_link_libraries(${PROJECT_NAME}
            uv_a
            userenv
            ws2_32
            iphlpapi
            shlwapi
            -lssp)
else()
    target_link_libraries(${PROJECT_NAME}
            uv_a
            pthread
            dl
            sqlite)
endif()
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(${PROJECT_NAME} stdc++fs)
endif()
target_link_libraries(${PROJECT_NAME} z)