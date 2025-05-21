/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_CURVEGROUPHANDLER_H
#define PROFILER_SERVER_CURVEGROUPHANDLER_H
#include "CurveRepo.h"
#include "IERequestHandler.h"
#include "IEProtocolResquest.h"
#include "IEProtocolResponse.h"

namespace Dic::Module::IE {
class CurveGroupHandler : public IERequestHandler {
public:
    CurveGroupHandler()
    {
        moduleName = MODULE_IE;
    }
    ~CurveGroupHandler() override = default;
    bool HandleRequest(std::unique_ptr<Dic::Protocol::Request> requestPtr) override;
protected:
    std::shared_ptr<CurveRepo> repo = std::make_shared<CurveRepo>();
};
}
#endif // PROFILER_SERVER_CURVEGROUPHANDLER_H
