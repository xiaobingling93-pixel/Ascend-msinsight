cmake_minimum_required(VERSION 3.16)
project(server_edge)

aux_source_directory(./ SERVER_BIN_SRC)

list(APPEND SERVER_SRC_LIST
        ${SERVER_BIN_SRC}
        ${U_SOCKETS_SRC_LIST}
        ${DIC_UTILS_SRC}
        ${DIC_PROTOCOL_SRC}
        ${DIC_SCENE_SRC_LIST}
        ${DIC_SERVER_SRC}
        ${DIC_CHANNEL_SRC})

add_executable(${PROJECT_NAME} ${SERVER_SRC_LIST})

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_link_libraries(${PROJECT_NAME}
            uv_a
            UserEnv
            ws2_32
            Iphlpapi
            shlwapi
            sqlite
            -lssp)
else()
    target_link_libraries(${PROJECT_NAME}
            uv_a
            pthread
            dl
            sqlite)
endif()