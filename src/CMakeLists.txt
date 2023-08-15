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

# src scene
include_directories(${SRC_HOME_DIR}/scene)
include_directories(${SRC_HOME_DIR}/scene/handler)
include_directories(${SRC_HOME_DIR}/scene/handler/global)
include_directories(${SRC_HOME_DIR}/scene/core)
include_directories(${SRC_HOME_DIR}/scene/core/database)
include_directories(${SRC_HOME_DIR}/scene/core/parser)
include_directories(${SRC_HOME_DIR}/scene/core/parser/trace)

include_directories(${SRC_HOME_DIR}/scene/core/hdc)
include_directories(${SRC_HOME_DIR}/scene/core/hdc/common)
include_directories(${SRC_HOME_DIR}/scene/handler/ascend)

# scene for server
aux_source_directory(${SRC_HOME_DIR}/scene DIC_SCENE_SRC)
aux_source_directory(${SRC_HOME_DIR}/scene/handler DIC_SCENE_HANDLER_SRC)
aux_source_directory(${SRC_HOME_DIR}/scene/handler/global DIC_SCENE_HANDLER_GLOBAL_SRC)
aux_source_directory(${SRC_HOME_DIR}/scene/core DIC_SCENE_CORE_SRC)
aux_source_directory(${SRC_HOME_DIR}/scene/core/database DIC_SCENE_CORE_DATABASE_SRC)
aux_source_directory(${SRC_HOME_DIR}/scene/core/parser DIC_SCENE_CORE_PARSER_SRC)
aux_source_directory(${SRC_HOME_DIR}/scene/core/parser/trace DIC_SCENE_CORE_PARSER_TRACE_SRC)

aux_source_directory(${SRC_HOME_DIR}/scene/core/hdc DIC_SCENE_CORE_HDC_SRC)
aux_source_directory(${SRC_HOME_DIR}/scene/core/hdc/common DIC_SCENE_CORE_HDC_COMMON_SRC)
aux_source_directory(${SRC_HOME_DIR}/scene/handler/ascend DIC_SCENE_HANDLER_ASCEND_SRC)

list(APPEND DIC_SCENE_SRC_LIST
        ${DIC_SCENE_SRC}
        ${DIC_SCENE_HANDLER_SRC}
        ${DIC_SCENE_HANDLER_GLOBAL_SRC}
        ${DIC_SCENE_CORE_SRC}
        ${DIC_SCENE_CORE_DATABASE_SRC}
        ${DIC_SCENE_CORE_PARSER_SRC}
        ${DIC_SCENE_CORE_PARSER_TRACE_SRC}
        ${DIC_SCENE_CORE_HDC_SRC}
        ${DIC_SCENE_CORE_HDC_COMMON_SRC}
        ${DIC_SCENE_HANDLER_ASCEND_SRC})

add_subdirectory(entry)