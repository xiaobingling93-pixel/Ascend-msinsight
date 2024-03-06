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
target_link_libraries(${PROJECT_NAME}
        sqlite)

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_link_libraries(${PROJECT_NAME}
            -static
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
            dl)
endif()