/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_IMPORT_ACTION_HANDLER_H
#define PROFILER_SERVER_IMPORT_ACTION_HANDLER_H

#include <set>
#include <regex>
#include "TimelineRequestHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
class ImportActionHandler : public TimelineRequestHandler {
public:
    ImportActionHandler()
    {
        command = Protocol::REQ_RES_IMPORT_ACTION;
    };
    ~ImportActionHandler() override = default;
    void HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;

private:
    static bool curIsCluster;

    static void SetParseCallBack(const std::string &token);
    static void ParseEndCallBack(const std::string &token, const std::string &fileId, bool result);
    static void ParseClusterEndProcess(const std::string token, std::string result);
    static void SearchMetaData(const std::string &fileId, std::vector<std::unique_ptr<UnitTrack>> &metaData);
    static std::string GetFileId(const std::string &filePath);
    static bool CheckIsCluster(const std::string &filePath);
    std::vector<std::string> FindTraceFile(const std::string &path);
    std::vector<std::string> FindAllTraceFile(const std::vector<std::string> &pathList);
    bool IsJsonValid(const std::string &fileName);
    void FindAscendFolder(const std::string &path, std::vector<std::string> &traceFiles);
    std::vector<std::pair<std::string, std::string>> GetTraceFiles(const std::vector<std::string> &pathList,
                                                                   ImportActionResBody &body);
    static void SendParseSuccessEvent(const std::string &token, const std::string &fileId);
    static void SendParseFailEvent(const std::string &token, const std::string &fileId);

    bool HasMemoryFile(const std::string& path);
    const std::string traceViewFile = "trace_view.json";
    const std::string memoryOperatorFile = "operator_memory.csv";
    const std::string memoryRecordFile = "memory_record.csv";
    const std::string traceViewReg = R"((trace_view|msprof_[0-9]{1,4}_[0-9]{1,4})\.json$)";
    const std::string memoryOperatorReg = R"((operator_memory|msprof_[0-9]{1,4}_[0-9]{1,4})\.csv$)";
    const std::string memoryRecordReg = R"((memory_record|msprof_[0-9]{1,4}_[0-9]{1,4})\.csv$)";
    void SetBaseActionOfResponse(const std::map<std::string, std::vector<std::string>>& rankListMap,
                                 ImportActionResponse &response);
    static std::vector<MemorySuccess> hasMemory;
    static void ParseMemoryEndProcess(const std::string token);
    static void ParseOperatorEndProcess(const std::string token, const std::string &fileId, bool result);
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_IMPORT_ACTION_HANDLER_H
