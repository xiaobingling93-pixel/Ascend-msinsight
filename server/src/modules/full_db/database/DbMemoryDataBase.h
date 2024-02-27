/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_DBMEMORYDATABASE_H
#define PROFILER_SERVER_DBMEMORYDATABASE_H

#include <map>
#include "VirtualMemoryDataBase.h"
#include "TimelineProtocolEvent.h"

namespace Dic {
namespace Module {
namespace FullDb {
class DbMemoryDataBase : public Memory::VirtualMemoryDataBase {
public:
    explicit DbMemoryDataBase(std::mutex &sqlMutex) : Memory::VirtualMemoryDataBase(sqlMutex) {};
    virtual ~DbMemoryDataBase() {};

    bool QueryOperatorDetail(Protocol::MemoryOperatorParams &requestParams,
                             std::vector<Protocol::MemoryTableColumnAttr> &columnAttr,
                             std::vector<Protocol::MemoryOperator> &opDetails);
    bool QueryMemoryView(Protocol::MemoryComponentParams &requestParams, Protocol::MemoryViewData &operatorBody);
    bool QueryOperatorsTotalNum(Protocol::MemoryOperatorParams &requestParams, int64_t &totalNum);
    bool QueryOperatorSize(double &min, double &max);
    static void ParserEnd(std::string rankId, bool result);
    static void ParseCallBack(const std::string &token, const std::string &fileId, bool result,
                              const std::string &msg);
    std::map<std::string, Protocol::MemorySuccess> GetRanks();

private:
    static std::map<std::string, Protocol::MemorySuccess> ranks;
    bool isCluster;
};

}
}
}

#endif // PROFILER_SERVER_DBMEMORYDATABASE_H
