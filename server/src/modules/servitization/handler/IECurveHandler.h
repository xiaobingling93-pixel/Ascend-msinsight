/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_IECURVEHANDLER_H
#define PROFILER_SERVER_IECURVEHANDLER_H
#include "CurveRepo.h"
#include "IERequestHandler.h"
#include "IEProtocolResquest.h"
#include "IEProtocolResponse.h"

namespace Dic::Module::IE {
class IECurveHandler : public IERequestHandler {
public:
    IECurveHandler()
    {
        moduleName = MODULE_IE;
    }
    ~IECurveHandler() override = default;
    bool HandleRequest(std::unique_ptr<Dic::Protocol::Request> requestPtr) override;

protected:
    std::shared_ptr<CurveRepo> repo = std::make_shared<CurveRepo>();

    void
    QueryDatasByCols(const IEUsageViewParamsRequest &request, IEUsageViewResponse &response,
                     std::vector<ColumnAtt> &atts);
};
}
#endif // PROFILER_SERVER_IECURVEHANDLER_H
