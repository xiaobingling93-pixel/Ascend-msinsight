/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_PROTOCOL_REQUEST_BUILDER_H
#define DATA_INSIGHT_CORE_PROTOCOL_REQUEST_BUILDER_H

#include <memory>
#include "ProtocolEnum.h"
#include "IdBuilder.h"

namespace Dic {
namespace Protocol {
class RequestBuilder {
public:
    template <class REQUEST>
    static inline std::unique_ptr<REQUEST> Build(const std::string &tokenStr, const SceneType &sceneType, int cbId = -1)
    {
        std::unique_ptr<REQUEST> requestPtr = std::make_unique<REQUEST>();
        requestPtr->token = tokenStr;
        requestPtr->id = IdBuilder::RequestIdBuilder().Build();
        requestPtr->scene = sceneType;
        if (cbId >= 0) {
            requestPtr->resultCallbackId = cbId;
        }
        return requestPtr;
    }
};
} // end of namespace Client
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_CLIENT_REQUEST_BUILDER_H
