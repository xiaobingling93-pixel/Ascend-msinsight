/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Scene request handler declaration
 */

#ifndef DATA_INSIGHT_CORE_SCENE_REQUEST_HANDLER_H
#define DATA_INSIGHT_CORE_SCENE_REQUEST_HANDLER_H

#include "Protocol.h"

namespace Dic {
namespace Scene {
using namespace Dic::Protocol;
class SceneRequestHandler {
public:
    SceneRequestHandler() = default;
    virtual ~SceneRequestHandler() = default;
    virtual const std::string GetError();
    virtual void HandleRequest(std::unique_ptr<Request> requestPtr) = 0;

protected:
    void SetBaseResponse(const Request &request, Response &response) const;
    void SetResponseResult(Response &response, bool result, const std::string &errorMsg = "",
                           const ErrorCode &errorCode = ErrorCode::UNKNOW_ERROR) const;

    std::string command;
    std::string error;
    SceneType sceneType = SceneType::UNKNOWN;
};
} // end of namespace Scene
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_SCENE_REQUEST_HANDLER_H
