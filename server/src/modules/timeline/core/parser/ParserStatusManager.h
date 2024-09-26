/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_PARSER_STATUS_MANAGER_H
#define PROFILER_SERVER_PARSER_STATUS_MANAGER_H

#include <map>
#include <memory>
#include <mutex>
#include "vector"
#include "condition_variable"

namespace Dic {
namespace Module {
namespace Timeline {
enum class ParserStatus {
    UN_KNOW = 0,
    INIT, // 未开始解析
    RUNNING, // 解析中
    FINISH, // 解释完成
    FINISH_ALL, // 解释完成
    TERMINATE, // 终止解析
};

class ParserStatusManager {
public:
    static ParserStatusManager &Instance();
    ParserStatusManager(const ParserStatusManager &) = delete;
    ParserStatusManager &operator=(const ParserStatusManager &) = delete;
    ParserStatusManager(ParserStatusManager &&) = delete;
    ParserStatusManager &operator=(ParserStatusManager &&) = delete;

    void SetParserStatus(const std::string &fileId, ParserStatus status);
    void ClearParserStatus(const std::string &fileId);
    void ClearAllParserStatus();
    ParserStatus GetParserStatus(const std::string &fileId);
    ParserStatus GetClusterParserStatus();
    bool SetRunningStatus(const std::string &fileId);
    bool SetFinishStatus(const std::string &fileId);
    void SetAllTerminateStatus();
    // return old status
    ParserStatus SetTerminateStatus(const std::string &fileId);
    void SetClusterParseStatus(ParserStatus parserStatus);
    void WaitAllFinished(const std::vector<std::string> &fileIds);
    bool IsAllFinished(std::string &notFinishTask);
    bool IsFinished(const std::string &fileId);
    bool IsKernelAndMemoryFinished(const std::string &fileId);
    void SetPendingStatus(const std::string &fileId,
        const std::pair<ProjectTypeEnum, std::vector<std::string>> &filePathPair);
    std::pair<ProjectTypeEnum, std::vector<std::string>> QueryPendingFilePath(const std::string &fileId);

private:
    ParserStatusManager() = default;
    ~ParserStatusManager() = default;

    std::mutex mutex;
    std::map<std::string, ParserStatus> statusMap;
    std::map<std::string, std::pair<ProjectTypeEnum, std::vector<std::string>>> pendingRankAndFilePathMap;
    ParserStatus clusterParseStatus;
    std::condition_variable parseCv;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_PARSER_STATUS_MANAGER_H
