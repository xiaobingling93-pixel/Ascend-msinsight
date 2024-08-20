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
        std::string GetGroup(OperatorDetailInfoRes &data);
        std::vector<Protocol::OperatorDetailCmpInfoRes> CalCompareInfo(int64_t &total,
            std::vector<Protocol::OperatorDetailInfoRes> &baseDbData,
            std::vector<Protocol::OperatorDetailInfoRes> &cmpDbData, int64_t pageSize, int64_t current);
        void dealResGetDiff(std::vector<Protocol::OperatorDetailCmpInfoRes> &res);
        void ProcessDataToMuiMap(std::vector<Protocol::OperatorDetailInfoRes> datFromDb,
                                 std::set<std::string> infoKey,
                                 std::multimap<std::string, Protocol::OperatorDetailInfoRes> multiDataMap);
        std::vector<Protocol::OperatorDetailCmpInfoRes> GetFixNumDiffCmpData(
            std::vector<Protocol::OperatorDetailCmpInfoRes> &datailData, const int64_t pageSize,
            const int64_t current);
    };
}

#endif // PROFILER_SERVER_QUERYOPDETAILINFOHANDLER_H
