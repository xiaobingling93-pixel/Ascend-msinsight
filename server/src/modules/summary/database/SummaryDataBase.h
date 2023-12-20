/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_COMMUNICATION_DATABASE_H
#define PROFILER_SERVER_COMMUNICATION_DATABASE_H

#include "Database.h"
#include "Protocol.h"
#include "SummaryDef.h"
#include "SummaryProtocolRequest.h"
#include "SummaryProtocolResponse.h"
#include "OperatorProtocolRequest.h"
#include "OperatorProtocolResponse.h"

namespace Dic {
namespace Module {
namespace Summary {
class SummaryDataBase : public Database {
public:
    explicit SummaryDataBase(std::mutex &sqlMutex);
    ~SummaryDataBase() override;

    bool SetConfig();
    bool CreateTable();
    bool InitStmt();
    void ReleaseStmt();

    void InsertKernelDetailList(std::vector<Kernel> kernelVec);
    void InsertKernelDetail(Kernel kernel);
    void SaveKernelDetail();
    bool HasParseKernelFile(const std::string& kernelFile);
    void AddParseKernelFile(const std::string& kernelFile);
    void ClearParseKernelFile();

    bool QueryComputeDetailHandler(Protocol::ComputeDetailParams params,
                                   std::vector<Protocol::ComputeDetail> &computeDetails);
    bool QueryGetTotalNum(std::string name, int64_t &totalNum);

    bool QueryCommDetailHandler(Protocol::CommunicationDetailParams params,
                                std::vector<Protocol::CommunicationDetail> &computeDetails);

    bool QueryOperatorDurationInfo(Protocol::OperatorDurationReqParams &reqParams, Protocol::QueryType type,
        std::vector<Protocol::OperatorDurationRes> &datas);

    bool QueryOperatorStatisticInfo(Protocol::OperatorStatisticReqParams &reqParams,
        Protocol::OperatorStatisticInfoResponse &response);

    bool QueryOperatorDetailInfo(Protocol::OperatorStatisticReqParams &reqParams,
        Protocol::OperatorDetailInfoResponse& response);

    bool QueryOperatorMoreInfo(Protocol::OperatorMoreInfoReqParams &reqParams,
        Protocol::OperatorMoreInfoResponse& response);

private:
    std::mutex &mutex;
    const std::string kernelTable = "kernel_detail";
    bool hasInitStmt = false;
    sqlite3_stmt *insertKernelStmt = nullptr;
    const int cacheSize = 1000;
    const int maxCategorySize = 50;
    std::vector<Kernel> kernelCache;
    std::vector<std::string> kernelFiles = {};

    sqlite3_stmt *GetKernelStmt(uint64_t paramLen);
    std::string GenComputeSql(Protocol::ComputeDetailParams request);
    std::string GetCommSql(Protocol::CommunicationDetailParams request);

    std::string GenerateQueryCategoryDurationSql(Protocol::OperatorDurationReqParams &reqParams);

    std::string GenerateQueryComputeUnitDurationSql(Protocol::OperatorDurationReqParams &reqParams);

    std::string GenerateQueryStatisticSql(Protocol::OperatorStatisticReqParams &reqParams);

    std::string GenerateQueryDetailSql(Protocol::OperatorStatisticReqParams &reqParams);

    std::string GenerateQueryMoreInfoSql(Protocol::OperatorMoreInfoReqParams &reqParams);

    bool QueryStatisticTotalNum(Protocol::OperatorStatisticReqParams &reqParams, int64_t &total);

    bool QueryDetailTotalNum(Protocol::OperatorStatisticReqParams &reqParams, int64_t &total);

    bool QueryMoreInfoTotalNum(Protocol::OperatorMoreInfoReqParams &reqParams, int64_t &total);
};

} // end of namespace Summary
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_SUMMARY_DATABASE_H
