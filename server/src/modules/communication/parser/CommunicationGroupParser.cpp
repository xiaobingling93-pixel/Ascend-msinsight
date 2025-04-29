/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "CommunicationGroupParser.h"
#include "ClusterDef.h"
#include "ServerLog.h"
#include "FileUtil.h"
#include "SafeFile.h"

namespace Dic {
namespace Module {
namespace Communication {
std::vector<CommGroupParallelInfo> CommunicationGroupParser::GetGroupFromParallelInfo(const json_t &json)
{
    auto parallelInfoList = json["comm_group_parallel_info"].GetArray();
    std::vector<CommGroupParallelInfo> res;
    for (const auto &item: parallelInfoList) {
        CommGroupParallelInfo info;
        info.type = JsonUtil::GetString(item, "type");
        info.groupId = JsonUtil::GetString(item, "group_id");
        info.groupIdHash = JsonUtil::GetString(item, "group_name");
        info.pgName = JsonUtil::GetString(item, "pg_name");
        info.rankSet = JsonUtil::GetVector<std::string>(item, "rank_set");
        info.rankSetStr = StringUtil::JoinNumberStrWithParenthesesByOrder(info.rankSet);
        res.push_back(info);
    }
    return res;
}

std::vector<CommGroupParallelInfo> CommunicationGroupParser::GetGroupFromP2pAndCollective(const json_t &json)
{
    // 分别从collective和p2p中读取数据
    std::vector<CommGroupParallelInfo> res;
    if (JsonUtil::IsJsonKeyValid(json, "p2p")) {
        auto p2p = json["p2p"].GetArray();
        for (const auto &item: p2p) {
            CommGroupParallelInfo info;
            info.type = "p2p";
            info.rankSet = JsonUtil::JsonToVector(item.GetArray());
            info.rankSetStr = StringUtil::JoinNumberStrWithParenthesesByOrder(info.rankSet);
            res.push_back(info);
        }
    }
    if (JsonUtil::IsJsonKeyValid(json, "collective")) {
        auto collectiveJson = json["collective"].GetArray();
        for (const auto &item: collectiveJson) {
            CommGroupParallelInfo info;
            info.type = "collective";
            info.rankSet = JsonUtil::JsonToVector(item.GetArray());
            info.rankSetStr = StringUtil::JoinNumberStrWithParenthesesByOrder(info.rankSet);
            res.push_back(info);
        }
    }
    if (res.empty()) {
        Server::ServerLog::Warn("Get group from p2p and collective return empty.");
    }
    return res;
}

std::vector<CommGroupParallelInfo> CommunicationGroupParser::ParseCommunicationGroupByText(
    const std::string &fileContent)
{
    try {
        std::string errorStr;
        std::optional<document_t> jsonInfo = JsonUtil::TryParse<kParseNumbersAsStringsFlag>(fileContent, errorStr);
        if (!errorStr.empty() || !jsonInfo.has_value()) {
            Server::ServerLog::Error("Try to parse communication group failed, error is ", errorStr);
            return {};
        }
        // 如果数据中存在comm_group_parallel_info字段，则以该字段为准（该字段会囊括p2p和collective内容，且信息更全）
        if (JsonUtil::IsJsonArray(jsonInfo.value(), "comm_group_parallel_info")) {
            return GetGroupFromParallelInfo(jsonInfo.value());
        } else {
            // 数据中没有comm_group_parallel_info字段，则表示数据较老，只能从p2p和collective中获取内容
            return GetGroupFromP2pAndCollective(jsonInfo.value());
        }
    } catch (std::exception &e) {
        // 异常解析失败返回空列表
        Server::ServerLog::Error("Parse communication group failed by system exception (%s)", e.what());
        return {};
    }
}

std::vector<CommGroupParallelInfo> CommunicationGroupParser::ParseCommunicationGroup(const std::string &selectedPath)
{
    const std::string &filePath = FileUtil::PathPreprocess(selectedPath);
    std::ifstream communicationGroup = OpenReadFileSafely(filePath, std::ios::binary);
    if (communicationGroup.good()) {
        auto start = std::chrono::high_resolution_clock::now();
        std::string fileContent;
        std::copy(std::istream_iterator<unsigned char>(communicationGroup), std::istream_iterator<unsigned char>(),
                  back_inserter(fileContent));
        std::vector<CommGroupParallelInfo> res = ParseCommunicationGroupByText(fileContent);
        auto end = std::chrono::high_resolution_clock::now();
        Server::ServerLog::Info("End parse communication group file data into db ,file:", filePath, ",cost time:",
            (end - start).count());
        return res;
    } else {
        Server::ServerLog::Error("Parse communication group file fail, path:", filePath);
        return {};
    }
}
}
}
}