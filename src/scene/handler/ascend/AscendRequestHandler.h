//
// * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
//

#ifndef PROFILER_SERVER_ASCEND_REQUEST_HANDLER_H
#define PROFILER_SERVER_ASCEND_REQUEST_HANDLER_H

#include "SceneRequestHandler.h"

namespace Dic {
namespace Scene {
class AscendRequestHandler : public SceneRequestHandler {
public:
    AscendRequestHandler()
    {
        sceneType = Protocol::SceneType::ASCEND;
    }
    ~AscendRequestHandler() override = default;
    void HandleRequest(std::unique_ptr<Dic::Protocol::Request> requestPtr) override {}
};
} // end of namespace Scene
} // end of namespace Dic

#endif // PROFILER_SERVER_ASCEND_REQUEST_HANDLER_H
