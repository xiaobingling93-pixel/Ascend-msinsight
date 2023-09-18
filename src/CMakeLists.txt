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
include_directories(${SRC_HOME_DIR}/modules/global/core)
include_directories(${SRC_HOME_DIR}/modules/global/handler)
include_directories(${SRC_HOME_DIR}/modules/global/protocol)
include_directories(${SRC_HOME_DIR}/modules/timeline)
include_directories(${SRC_HOME_DIR}/modules/timeline/core)
include_directories(${SRC_HOME_DIR}/modules/timeline/core/database)
include_directories(${SRC_HOME_DIR}/modules/timeline/core/parser)
include_directories(${SRC_HOME_DIR}/modules/timeline/handler)
include_directories(${SRC_HOME_DIR}/modules/timeline/protocol)

# modules for server
aux_source_directory(${SRC_HOME_DIR}/modules DIC_MODULES_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/base DIC_MODULES_BASE_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/base/core DIC_MODULES_BASE_CORE_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/base/handler DIC_MODULES_BASE_HANDLER_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/base/protocol DIC_MODULES_BASE_PROTOCOL_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/global DIC_MODULES_GLOBAL_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/global/core DIC_MODULES_GLOBAL_CORE_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/global/handler DIC_MODULES_GLOBAL_HANDLER_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/global/protocol DIC_MODULES_GLOBAL_PROTOCOL_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/timeline  DIC_MODULES_TIMELINE_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/timeline/core DIC_MODULES_TIMELINE_CORE_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/timeline/core/database DIC_MODULES_TIMELINE_CORE_DATABASE_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/timeline/core/parser DIC_MODULES_TIMELINE_CORE_PARSER_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/timeline/handler DIC_MODULES_TIMELINE_HANDLER_SRC)
aux_source_directory(${SRC_HOME_DIR}/modules/timeline/protocol DIC_MODULES_TIMELINE_PROTOCOL_SRC)

list(APPEND DIC_MODULES_SRC_LIST
        ${DIC_MODULES_SRC}
        ${DIC_MODULES_BASE_SRC}
        ${DIC_MODULES_BASE_CORE_SRC}
        ${DIC_MODULES_BASE_HANDLER_SRC}
        ${DIC_MODULES_BASE_PROTOCOL_SRC}
        ${DIC_MODULES_GLOBAL_SRC}
        ${DIC_MODULES_GLOBAL_CORE_SRC}
        ${DIC_MODULES_GLOBAL_HANDLER_SRC}
        ${DIC_MODULES_GLOBAL_PROTOCOL_SRC}
        ${DIC_MODULES_TIMELINE_SRC}
        ${DIC_MODULES_TIMELINE_CORE_SRC}
        ${DIC_MODULES_TIMELINE_CORE_DATABASE_SRC}
        ${DIC_MODULES_TIMELINE_CORE_PARSER_SRC}
        ${DIC_MODULES_TIMELINE_HANDLER_SRC}
        ${DIC_MODULES_TIMELINE_PROTOCOL_SRC})

add_subdirectory(entry)