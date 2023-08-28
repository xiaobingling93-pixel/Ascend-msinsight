/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_PROTOCOL_ENTITY_UTIL_H
#define DATA_INSIGHT_CORE_PROTOCOL_ENTITY_UTIL_H

#include "GlobalDefs.h"
#include "ProtocolBase.h"
#include "ProtocolEntity.h"
#include "ProtocolEnumUtil.h"

namespace Dic {
namespace Protocol {
class ProtocolUtil {
public:
    // entity
    static void SetGlobalConfigJson(const GlobalConfig &config, json_t &jsonGlobalConfig);
    static void SetGlobalConfigStruct(const json_t &jsonGlobalConfig, GlobalConfig &config);
    // request
    static void SetRequestJsonBaseInfo(const Request &request, json_t &json);
    static bool SetRequestBaseInfo(Request &request, const json_t &json);
    // response
    static void SetResponseJsonBaseInfo(const Response &response, json_t &json);
    static bool SetResponseBaseInfo(Response &response, const json_t &json);
    // event
    static void SetEventJsonBaseInfo(const Event &event, json_t &json);
    static bool SetEventBaseInfo(Event &event, const json_t &json);

private:
    ProtocolUtil() = default;
    ~ProtocolUtil() = default;
};
} // end of namespace Protocol
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_PROTOCOL_ENTITY_UTIL_H
