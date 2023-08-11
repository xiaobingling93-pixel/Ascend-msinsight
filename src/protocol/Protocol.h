/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol declaration
 */

#ifndef DIC_PROTOCOL_H
#define DIC_PROTOCOL_H

#include "ProtocolDefs.h"
#include "ProtocolBase.h"
#include "ProtocolEnum.h"
#include "ProtocolEntity.h"
#include "ProtocolEnumUtil.h"
#include "ProtocolUtil.h"
#include "ProtocolRequest.h"
#include "ProtocolResponse.h"
#include "ProtocolEvent.h"
#include "RequestManager.h"
#include "ResponseManager.h"
#include "SceneErrorCode.h"
#include "EventManager.h"
#include "ProtocolMessageBuffer.h"

namespace Dic {
namespace Protocol {
using namespace Dic::Scene;
// scene map
const std::map<SceneType, std::string> SCENE_MAP = {
    { SceneType::UNKNOWN, SCENE_UNKNOWN }, { SceneType::GLOBAL, SCENE_GLOBAL }, { SceneType::DATABASE, SCENE_DATABASE },
    { SceneType::TOOL, SCENE_TOOL },       { SceneType::LOG, SCENE_LOG },       { SceneType::HARMONY, SCENE_HARMONY }
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