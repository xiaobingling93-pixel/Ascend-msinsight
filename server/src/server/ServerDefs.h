/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_SERVER_DEFS_H
#define DATA_INSIGHT_CORE_SERVER_DEFS_H

#include "App.h"

namespace Dic {
namespace Server {
struct WsUserData {
    std::string reqUrl;
    std::string sid;
    void *arg;
};
using WsChannel = uWS::WebSocket<false, true, WsUserData>;
} // end of namespace Server
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_SERVER_DEFS_H
