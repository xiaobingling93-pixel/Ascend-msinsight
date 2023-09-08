cmake_minimum_required(VERSION 3.16)

set(SRC_HOME_DIR ${HOME_DIR}/src)

# usockets definitions
add_compile_definitions("LIBUS_NO_SSL")
add_compile_definitions("UWS_NO_ZLIB")
add_compile_definitions("LIBUS_USE_LIBUV")

# src defs
include_directories(${SRC_HOME_DIR}/defs)

# src utils
include_directories(${SRC_HOME_DIR}/utils)
aux_source_directory(${SRC_HOME_DIR}/utils DIC_UTILS_SRC)

# src channel
include_directories(${SRC_HOME_DIR}/channel)
aux_source_directory(${SRC_HOME_DIR}/channel DIC_CHANNEL_SRC)

# src server
include_directories(${SRC_HOME_DIR}/server)
aux_source_directory(${SRC_HOME_DIR}/server DIC_SERVER_SRC)

# src protocol
include_directories(${SRC_HOME_DIR}/protocol)
aux_source_directory(${SRC_HOME_DIR}/protocol DIC_PROTOCOL_SRC)

# src modules
include_directories(${SRC_HOME_DIR}/modules)
include_directories(${SRC_HOME_DIR}/modules/base)
include_directories(${SRC_HOME_DIR}/modules/base/core)
include_directories(${SRC_HOME_DIR}/modules/base/handler)
include_directories(${SRC_HOME_DIR}/modules/base/protocol)
include_directories(${SRC_HOME_DIR}/modules/global)
include_directories(${SRC_HOME_DIR}/modules/global/handler)
include_directories(${SRC_HOME_DIR}/modules/global/protocol)
include_directories(${SRC_HOME_DIR}/modules/timeline)
include_directories(${SRC_HOME_DIR}/modules/timeline/core)
include_directories(${SRC_HOME_DIR}/modules/timeline/core/database)
include_directories(${SRC_HOME_DIR}/modules/timeline/core/parser)
include_directories(${SRC_HOME_DIR}/modules/timeline/handler)
include_directories(${SRC_HOME_DIR}/modules/timeline/protocol)
include_directories(${SRC_HOME_DIR}/modules/summary)
include_directories(${SRC_HOME_DIR}/modules/summary/protocol)
include_directories(${SRC_HOME_DIR}/modules/summary/handler)
include_directories(${SRC_HOME_DIR}/modules/summary/database)
include_directories(${SRC_HOME_DIR}/modules/summary/parser)
include_directories(${SRC_HOME_DIR}/modules/memory)
include_directories(${SRC_HOME_DIR}/modules/memory/database)
include_directories(${SRC_HOME_DIR}/modules/memory/handler)
include_directories(${SRC_HOME_DIR}/modules/memory/parser)
include_directories(${SRC_HOME_DIR}/modules/memory/protocol)
include_directories(${SRC_HOME_DIR}/modules/communication)
include_directories(${SRC_HOME_DIR}/modules/communication/protocol)
include_directories(${SRC_HOME_DIR}/modules/communication/handler)

# modules for server
aux_source_directory(${SRC_HOME_DIR}/modules DIC_MODULES_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/base DIC_MODULES_BASE_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/base/core DIC_MODULES_BASE_CORE_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/base/handler DIC_MODULES_BASE_HANDLER_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/base/protocol DIC_MODULES_BASE_PROTOCOL_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/global DIC_MODULES_GLOBAL_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/global/handler DIC_MODULES_GLOBAL_HANDLER_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/global/protocol DIC_MODULES_GLOBAL_PROTOCOL_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/timeline  DIC_MODULES_TIMELINE_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/timeline/core DIC_MODULES_TIMELINE_CORE_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/timeline/core/database DIC_MODULES_TIMELINE_CORE_DATABASE_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/timeline/core/parser DIC_MODULES_TIMELINE_CORE_PARSER_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/timeline/handler DIC_MODULES_TIMELINE_HANDLER_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/timeline/protocol DIC_MODULES_TIMELINE_PROTOCOL_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/summary/protocol DIC_MODULES_SUMMARY_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/summary/handler DIC_MODULES_SUMMARY_HANDLER_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/summary DIC_MODULES_SUMMARY_PROTOCOL_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/summary/database DIC_MODULES_SUMMARY_DATABASE_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/summary/parser DIC_MODULES_SUMMARY_PARSER_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/memory  DIC_MODULES_MEMORY_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/memory/database DIC_MODULES_MEMORY_DATABASE_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/memory/handler DIC_MODULES_MEMORY_HANDLER_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/memory/parser DIC_MODULES_MEMORY_PARSER_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/memory/protocol DIC_MODULES_MEMORY_PROTOCOL_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/summary/protocol DIC_MODULES_COMMUNICATION_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/summary/handler DIC_MODULES_COMMUNICATION_HANDLER_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/summary DIC_MODULES_COMMUNICATION_PROTOCOL_SRC)

list(APPEND DIC_MODULES_SRC_LIST
        ${DIC_MODULES_SRC}
        ${DIC_MODULES_BASE_SRC}
        ${DIC_MODULES_BASE_CORE_SRC}
        ${DIC_MODULES_BASE_HANDLER_SRC}
        ${DIC_MODULES_BASE_PROTOCOL_SRC}
        ${DIC_MODULES_GLOBAL_SRC}
        ${DIC_MODULES_GLOBAL_HANDLER_SRC}
        ${DIC_MODULES_GLOBAL_PROTOCOL_SRC}
        ${DIC_MODULES_TIMELINE_SRC}
        ${DIC_MODULES_TIMELINE_CORE_SRC}
        ${DIC_MODULES_TIMELINE_CORE_DATABASE_SRC}
        ${DIC_MODULES_TIMELINE_CORE_PARSER_SRC}
        ${DIC_MODULES_TIMELINE_HANDLER_SRC}
        ${DIC_MODULES_TIMELINE_PROTOCOL_SRC}
        ${DIC_MODULES_SUMMARY_SRC}
        ${DIC_MODULES_SUMMARY_HANDLER_SRC}
        ${DIC_MODULES_SUMMARY_PROTOCOL_SRC}
        ${DIC_MODULES_SUMMARY_DATABASE_SRC}
        ${DIC_MODULES_SUMMARY_PARSER_SRC}
        ${DIC_MODULES_TIMELINE_PROTOCOL_SRC}
        ${DIC_MODULES_MEMORY_SRC}
        ${DIC_MODULES_MEMORY_DATABASE_SRC}
        ${DIC_MODULES_MEMORY_PARSER_SRC}
        ${DIC_MODULES_MEMORY_HANDLER_SRC}
        ${DIC_MODULES_MEMORY_PROTOCOL_SRC}
        ${DIC_MODULES_SUMMARY_PROTOCOL_SRC}
        ${DIC_MODULES_COMMUNICATION_SRC}
        ${DIC_MODULES_COMMUNICATION_HANDLER_SRC}
        ${DIC_MODULES_COMMUNICATION_PROTOCOL_SRC})

add_subdirectory(entry)
if (${_PROJECT_TYPE} MATCHES "test")
    enable_testing()
    set(CMAKE_C_FLAGS_PROFILE --coverage)
    set(CMAKE_CXX_FLAGS_PROFILE --coverage)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    add_subdirectory(${HOME_DIR}/third_party/googletest googletest.out EXCLUDE_FROM_ALL)
    add_subdirectory(test)
elseif ($ENV{DEV_TYPE} MATCHES "true")

    enable_testing()
    add_subdirectory(${HOME_DIR}/third_party/googletest googletest.out EXCLUDE_FROM_ALL)
    add_subdirectory(test)
endif()