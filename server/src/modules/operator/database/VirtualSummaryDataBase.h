/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SUMMARY_DATABASE_H
#define PROFILER_SERVER_SUMMARY_DATABASE_H

#include "Database.h"
#include "Protocol.h"
#include "SummaryDef.h"
#include "SummaryProtocolRequest.h"
#include "SummaryProtocolResponse.h"
#include "OperatorProtocolRequest.h"
#include "OperatorProtocolResponse.h"
#include "TableDefs.h"
#include "NumDefs.h"
#include "CommonDefs.h"

namespace Dic::Module::Summary {
class VirtualSummaryDataBase : public Database {
public:
    bool levelIsL0 = true;

    explicit VirtualSummaryDataBase(std::recursive_mutex &sqlMutex) : Database(sqlMutex) {};
    ~VirtualSummaryDataBase() override = default;

    virtual bool QueryComputeDetailHandler(Protocol::ComputeDetailParams params,
                                   std::vector<Protocol::ComputeDetail> &computeDetails) = 0;
    virtual bool QueryGetTotalNum(std::string name, int64_t &totalNum) = 0;

    virtual bool QueryCommDetailHandler(Protocol::CommunicationDetailParams params,
                                std::vector<Protocol::CommunicationDetail> &computeDetails) = 0;

    virtual bool QueryOperatorDurationInfo(Protocol::OperatorDurationReqParams &reqParams, Protocol::QueryType type,
                                   std::vector<Protocol::OperatorDurationRes> &datas) = 0;

    virtual bool QueryOperatorStatisticInfo(Protocol::OperatorStatisticReqParams &reqParams,
                                    Protocol::OperatorStatisticInfoResponse &response) = 0;

    virtual bool QueryOperatorDetailInfo(Protocol::OperatorStatisticReqParams &reqParams,
                                 Protocol::OperatorDetailInfoResponse& response) = 0;
    virtual bool QueryAllOperatorDetailInfo(Protocol::OperatorStatisticReqParams &reqParams,
                                            std::vector<Protocol::OperatorDetailInfoRes> &res,
                                            std::string &level) = 0;

    virtual bool QueryOperatorMoreInfo(Protocol::OperatorMoreInfoReqParams &reqParams,
                               Protocol::OperatorMoreInfoResponse& response) = 0;
    virtual bool QueryAllOperatorStatisticInfo(Protocol::OperatorStatisticReqParams &reqParams,
                                               std::vector<Protocol::OperatorStatisticInfoRes> &res) = 0;

    uint64_t QueryMinStartTime();
    static inline std::string GetFileIdFromCombinationId(const std::string& str)
    {
        auto len = MSPROF_PREFIX.length();
        if (str.length() <= len || str.compare(0, len, MSPROF_PREFIX) != 0) {
            return str;
        }

        auto index = str.find_last_of(MSPROF_CONNECT);
        if (index == std::string::npos) {
            return str;
        }

        return str.substr(len, index - len - 1);
    }

    static inline std::string GetDeviceIdFromCombinationId(const std::string& str)
    {
        auto len = MSPROF_PREFIX.length();
        if (str.length() <= len || str.compare(0, len, MSPROF_PREFIX) != 0) {
            return str;
        }

        auto index = str.find_last_of(MSPROF_CONNECT);
        if (index == std::string::npos) {
            return str;
        }

        return str.substr(index + MSPROF_CONNECT.length() - 1);
    }

    std::string OperatorGetLevel(const std::vector<Protocol::OperatorDetailInfoRes> &res)
    {
        std::string level;
        if (res.empty()) {
            level = levelIsL0 ? "l0" : "l1";
        } else if (res.at(0).inputShape.empty()) {
            level = "l0";
            levelIsL0 = true;
        } else {
            level = "l1";
            levelIsL0 = false;
        }
        return level;
    }
};
}

#endif // PROFILER_SERVER_SUMMARY_DATABASE_H
