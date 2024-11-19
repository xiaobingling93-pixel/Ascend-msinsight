/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_MEMORY_DATABASE_H
#define PROFILER_SERVER_MEMORY_DATABASE_H
#include "Protocol.h"
#include "VirtualMemoryDataBase.h"
#include "MemoryProtocolRespose.h"
#include "MemoryProtocolRequest.h"
#include "MemoryDef.h"

namespace Dic {
namespace Module {
namespace Memory {
using memoryLines = std::vector<std::vector<std::string>>;
using componentDtoVector = std::vector<Protocol::ComponentDto>;
class TextMemoryDataBase : public VirtualMemoryDataBase {
public:
    explicit TextMemoryDataBase(std::recursive_mutex &sqlMutex);
    ~TextMemoryDataBase() override;

    bool SetConfig();
    bool CreateTable();
    bool DropTable();
    bool InitStmt();
    void ReleaseStmt();

    void InsertOperatorDetailList(const std::vector<Operator> &eventList);
    void insertOperatorDetail(const Operator &event);
    void InsertRecordDetailList(const std::vector<Record> &eventList);
    void insertRecordDetail(const Record &event);
    void InsertStaticOpDetailList(const std::vector<StaticOp> &eventList);
    void insertStaticOpDetail(const StaticOp &event);

    bool QueryMemoryType(std::string &type, std::vector<std::string> &graphId) override;
    bool QueryMemoryResourceType(std::string &type) override;
    bool QueryOperatorDetail(Protocol::MemoryOperatorParams &requestParams,
                             std::vector<Protocol::MemoryTableColumnAttr> &columnAttr,
                             std::vector<Protocol::MemoryOperator> &opDetails) override;
    bool QueryMemoryView(Protocol::MemoryViewParams &requestParams,
                         Protocol::MemoryViewData &operatorBody, uint64_t offsetTime) override;

    bool QueryStaticOperatorList(Protocol::StaticOperatorListParams &requestParams,
                             std::vector<Protocol::MemoryTableColumnAttr> &columnAttr,
                             std::vector<Protocol::StaticOperatorItem> &opDetails) override;

    bool QueryStaticOperatorGraph(Protocol::StaticOperatorGraphParams &requestParams,
                             Protocol::StaticOperatorGraphItem &graphItem) override;

    void SaveRecordDetail();
    void SaveOperatorDetail();
    void SaveStaticOpDetail();

    bool QueryOperatorsTotalNum(Protocol::MemoryOperatorParams &requestParams, int64_t &totalNum) override;
    bool QueryStaticOperatorsTotalNum(Protocol::StaticOperatorListParams &requestParams, int64_t &totalNum) override;
    bool QueryOperatorSize(double &min, double &max, std::string rankId) override;
    bool QueryEntireOperatorTable(std::vector<Protocol::MemoryTableColumnAttr> &columnattr,
                                  std::vector<Protocol::MemoryOperator> &opDetails, std::string rankId,
                                  uint64_t offsetTime) override;
    bool QueryEntireStaticOperatorTable(Protocol::StaticOperatorListParams& requestParams,
                                        std::vector<Protocol::MemoryTableColumnAttr>& columnAttr,
                                        std::vector<Protocol::StaticOperatorItem>& opDetails) override;
    uint64_t QueryMinOperatorAllocationTime();
    uint64_t QueryMinRecordTimestamp();

    bool UpdateParseStatus(const std::string& status);
    bool HasFinishedParseLastTime();

private:
    const std::string operatorTable = "operator";
    const std::string recordTable = "record";
    const std::string staticOpTable = "static_op"; // 静态图表
    const std::string memoryParseStatus = "Memory files parsing status";
    const int exLength = 4;

    bool hasInitStmt = false;
    const uint32_t cacheSize = 100;
    std::vector<Operator> operatorCache;
    std::vector<Record> recordCache;
    std::vector<StaticOp> staticOpCache;

    sqlite3_stmt *insertOperatorStmt = nullptr;
    sqlite3_stmt *insertRecordStmt = nullptr;
    sqlite3_stmt *insertStaticOpStmt = nullptr;

    sqlite3_stmt *GetOperatorStmt(uint64_t paramLen);
    sqlite3_stmt *GetRecordStmt(uint64_t paramLen);
    sqlite3_stmt *GetStaticOpStmt(uint64_t paramLen);

    std::string GetOperatorSql(Protocol::MemoryOperatorParams &requestParams);
    std::string GetStaticOperatorSql(Protocol::StaticOperatorListParams &requestParams);
    std::string GetStaticGraphStartSql(Protocol::StaticOperatorGraphParams &requestParams);
    std::string GetStaticGraphEndSql(Protocol::StaticOperatorGraphParams &requestParams);

    const std::string COMPONENT_APP = "APP";
    const std::string COMPONENT_GE = "GE";
    const std::string COMPONENT_PTA = "PTA";
    const std::string COMPONENT_PTA_AND_GE = "PTA+GE";
};

} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_MEMORY_DATABASE_H
