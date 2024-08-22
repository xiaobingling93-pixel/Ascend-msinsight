/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERYOPDETAILINFOHANDLER_H
#define PROFILER_SERVER_QUERYOPDETAILINFOHANDLER_H

#include "ModuleRequestHandler.h"
#include "OperatorRequestHandler.h"
#include "OperatorProtocolRequest.h"
#include "OperatorProtocolResponse.h"

namespace Dic::Module::Operator {
    class QueryOpDetailInfoHandler : public OperatorRequestHandler {
    public:
        QueryOpDetailInfoHandler()
        {
            command = REQ_RES_OPERATOR_DETAIL_INFO;
        }

        ~QueryOpDetailInfoHandler() override = default;

        void HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
    private:
        bool HandleCompareDataRequest(OperatorDetailInfoRequest &request,
                                      OperatorDetailInfoResponse &response);
        bool HandleDetailDataRequest(OperatorDetailInfoRequest &request,
                                     OperatorDetailInfoResponse &response);
        std::vector<Protocol::OperatorDetailCmpInfoRes> GetCmpDataVec(
            std::vector<Protocol::OperatorDetailInfoRes> &baseDbData,
            std::vector<Protocol::OperatorDetailInfoRes> &cmpDbData);
        std::vector<Protocol::OperatorDetailCmpInfoRes> GetFixNumDiffCmpData(
            std::vector<Protocol::OperatorDetailCmpInfoRes> &datailData, const int64_t pageSize,
            const int64_t current, const std::string &order, const std::string &orderBy);
        void SortDataBynameAndStartTime(std::vector<Protocol::OperatorDetailInfoRes> &baseDbData,
                                std::vector<Protocol::OperatorDetailInfoRes> &cmpDbData);
        void FromatDatailData(Protocol::OperatorDetailCmpInfoRes &data);
    };
}

#endif // PROFILER_SERVER_QUERYOPDETAILINFOHANDLER_H
