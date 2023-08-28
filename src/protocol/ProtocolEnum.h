/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol Enums declaration
 */

#ifndef DIC_PROTOCOL_ENUM_H
#define DIC_PROTOCOL_ENUM_H

namespace Dic {
namespace Protocol {
// 每次左移一位
enum class ModuleType : int {
    UNKNOWN = -1,
    GLOBAL = 1,
    TIMELINE = 2,
};

enum class LinkType : int {
    SOCKET = 0,
    WEBSOCKET
};

} // end of namespace Protocol
} // end if namespace Dic

#endif // DIC_PROTOCOL_ENUM_H
