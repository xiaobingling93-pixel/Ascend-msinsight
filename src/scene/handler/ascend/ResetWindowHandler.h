//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
//

#ifndef PROFILER_SERVER_RESET_WINDOW_HANDLER_H
#define PROFILER_SERVER_RESET_WINDOW_HANDLER_H

#include "AscendRequestHandler.h"

namespace Dic {
namespace Scene {
class ResetWindowHandler : public AscendRequestHandler {
public:
    ResetWindowHandler()
    {
        command = Protocol::REQ_RES_RESET_WINDOW;
    };
    ~ResetWindowHandler() override = default;
    void HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // end of namespace Scene
} // end of namespace Dic


#endif // PROFILER_SERVER_RESET_WINDOW_HANDLER_H
