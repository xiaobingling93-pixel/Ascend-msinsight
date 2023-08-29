/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol declaration
 */

#ifndef DIC_PROTOCOL_H
#define DIC_PROTOCOL_H

#include "ProtocolDefs.h"
#include "ProtocolBase.h"
#include "ProtocolEnum.h"
#include "ProtocolEnumUtil.h"
#include "ProtocolUtil.h"
#include "TimelineProtocolEvent.h"
#include "ProtocolErrorCode.h"

namespace Dic {
namespace Protocol {
// module map
const std::map<ModuleType, std::string> MODULE_MAP = {
    {ModuleType::UNKNOWN, MODULE_UNKNOWN }, {ModuleType::GLOBAL, MODULE_GLOBAL }, {ModuleType::TIMELINE, MODULE_TIMELINE }
};

inline ErrorMessage MakeError(ErrorCode errorCode, const std::string &message)
{
    ErrorMessage errorMessage;
    errorMessage.code = static_cast<int>(errorCode);
    errorMessage.message = message;
    return errorMessage;
}
} // end of namespace Protocol
} // end if namespace Dic

#endif // DIC_PROTOCOL_H