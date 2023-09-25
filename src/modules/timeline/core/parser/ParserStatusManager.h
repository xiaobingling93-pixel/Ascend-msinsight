/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_PARSER_STATUS_MANAGER_H
#define PROFILER_SERVER_PARSER_STATUS_MANAGER_H

#include <map>
#include <memory>
#include <mutex>

namespace Dic {
namespace Module {
namespace Timeline {
enum class ParserStatus {
    UN_KNOW = 0,
    INIT, // 未开始解析
    RUNNING, // 解析中
    FINISH, // 解释完成
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
    bool SetRunningStatus(const std::string &fileId);
    bool SetFinishStatus(const std::string &fileId);
    // return old status
    ParserStatus SetTerminateStatus(const std::string &fileId);

private:
    ParserStatusManager() = default;
    ~ParserStatusManager() = default;

    std::mutex mutex;
    std::map<std::string, ParserStatus> statusMap;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_PARSER_STATUS_MANAGER_H
