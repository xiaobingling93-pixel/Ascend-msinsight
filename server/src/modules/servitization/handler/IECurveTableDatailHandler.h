/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_IECURVETABLEDATAILHANDLER_H
#define PROFILER_SERVER_IECURVETABLEDATAILHANDLER_H
#include "BaseDomain.h"
#include "CurveRepo.h"
#include "IEProtocolResponse.h"
#include "IERequestHandler.h"
namespace Dic::Module::IE {
    class IECurveTableDatailHandler : public IERequestHandler {
    public:
        IECurveTableDatailHandler()
        {
            moduleName = MODULE_IE;
        }
        ~IECurveTableDatailHandler() override = default;
        bool HandleRequest(std::unique_ptr<Dic::Protocol::Request> requestPtr) override;
    protected:
        std::shared_ptr<CurveRepo> repo = std::make_shared<CurveRepo>();
        void QueryViewData(IETableViewResponse &response, const PageQuery &query);
    };
}
#endif // PROFILER_SERVER_IECURVETABLEDATAILHANDLER_H
