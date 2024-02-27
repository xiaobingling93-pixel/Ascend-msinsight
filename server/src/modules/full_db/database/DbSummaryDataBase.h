/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_DBSUMMARYDATABASE_H
#define PROFILER_SERVER_DBSUMMARYDATABASE_H

#include "VirtualSummaryDataBase.h"
#include "OperatorProtocolRequest.h"
namespace Dic::Module::FullDb {
using namespace Dic::Protocol;
class DbSummaryDataBase : public Summary::VirtualSummaryDataBase {
public:
    explicit DbSummaryDataBase(std::mutex &sqlMutex) : Summary::VirtualSummaryDataBase(sqlMutex) {};
    virtual ~DbSummaryDataBase() {};

    bool QueryComputeDetailHandler(Protocol::ComputeDetailParams params,
                                   std::vector<Protocol::ComputeDetail> &computeDetails);
    bool QueryGetTotalNum(std::string name, int64_t &totalNum);
    bool QueryCommDetailHandler(Protocol::CommunicationDetailParams params,
                                std::vector<Protocol::CommunicationDetail> &commDetails);
    bool QueryOperatorDurationInfo(Protocol::OperatorDurationReqParams &reqParams, Protocol::QueryType type,
                                   std::vector<Protocol::OperatorDurationRes> &datas);
    bool QueryOperatorStatisticInfo(Protocol::OperatorStatisticReqParams &reqParams,
                                    Protocol::OperatorStatisticInfoResponse &response);
    bool QueryOperatorDetailInfo(Protocol::OperatorStatisticReqParams &reqParams,
                                 Protocol::OperatorDetailInfoResponse& response);
    bool QueryOperatorMoreInfo(Protocol::OperatorMoreInfoReqParams &reqParams,
                               Protocol::OperatorMoreInfoResponse& response);
    static void ParserEnd(const std::string &token, const std::string &fileId, bool result,
                          const std::string &msg);
private:
    bool QueryStatisticTotalNum(Protocol::OperatorStatisticReqParams &reqParams, int64_t &total);
    std::string GenerateQueryStatisticSql(Protocol::OperatorStatisticReqParams &reqParams);
    std::string GenComputeSql(const Protocol::ComputeDetailParams& request);
    static std::string GenerateQueryCategoryDurationSql(OperatorDurationReqParams &reqParams);
    static std::string GenerateQueryComputeUnitDurationSql(OperatorDurationReqParams &reqParams);
    bool QueryDetailTotalNum(OperatorStatisticReqParams &reqParams, int64_t &total);
    static std::string GenerateQueryDetailSql(OperatorStatisticReqParams &reqParams);
    bool QueryMoreInfoTotalNum(OperatorMoreInfoReqParams &reqParams, int64_t &total);
    std::string GenerateQueryMoreInfoSql(OperatorMoreInfoReqParams &reqParams);
    std::string GetCommSql(const CommunicationDetailParams& request);

    const int maxCategorySize = 50;

    void BindSqliteParam(sqlite3_stmt *stmt, OperatorMoreInfoReqParams &reqParams);
};

}
#endif // PROFILER_SERVER_DBSUMMARYDATABASE_H
