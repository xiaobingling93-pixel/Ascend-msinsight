/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "MetaDataParser.h"
#include "ServerLog.h"
#include "JsonUtil.h"
#include "FileReader.h"

namespace Dic::Module::Timeline {
std::vector<ParallelGroupInfo> MetaDataParser::ParserParallelGroupInfoByText(const std::string &text)
{
    std::vector<ParallelGroupInfo> res;
    if (text.empty()) {
        return res;
    }
    try {
        std::string error;
        auto groupInfoJson = JsonUtil::TryParse<kParseNumbersAsStringsFlag>(text, error);
        if (!error.empty()) {
            Server::ServerLog::Error("Fail to convert parallel group info, error: ", error);
            return res;
        }
        return ConvertGroupInfoJsonToObject(groupInfoJson.value());
    } catch (const std::exception &e) {
        Server::ServerLog::Error("Fail to parser parallel group info. Error: ", e.what());
        return res;
    }
}

std::vector<ParallelGroupInfo> MetaDataParser::ParserParallelGroupInfoByFilePath(const std::string &filePath)
{
    FileReader reader;
    std::string fileContext = reader.ReadJsonArray(filePath, 0, 0);
    std::vector<ParallelGroupInfo> res;
    if (fileContext.empty()) {
        Server::ServerLog::Error("Fail to read meta data file.");
        return res;
    }
    try {
        std::string error;
        auto metaDataJsonOpt = JsonUtil::TryParse<kParseNumbersAsStringsFlag>(fileContext, error);
        if (!error.empty()) {
            Server::ServerLog::Error("Fail to parser meta data file, error: ", error);
            return res;
        }
        if (!metaDataJsonOpt.value().IsObject()) {
            Server::ServerLog::Error("Fail to parser meta data file, data in wrong format.");
            return res;
        }
        if (metaDataJsonOpt.value().HasMember("parallel_group_info")) {
            document_t groupInfo;
            groupInfo.CopyFrom(metaDataJsonOpt.value()["parallel_group_info"], groupInfo.GetAllocator());
            res = ConvertGroupInfoJsonToObject(groupInfo);
        }
        return res;
    } catch (const std::exception &e) {
        Server::ServerLog::Error("Fail to parser meta data file context. Error: ", e.what());
        return res;
    }
}

std::vector<ParallelGroupInfo> MetaDataParser::ConvertGroupInfoJsonToObject(const document_t &json)
{
    std::vector<ParallelGroupInfo> res;
    if (!json.IsObject()) {
        Server::ServerLog::Error("Fail to convert parallel group info, data in wrong format.");
        return res;
    }
    for (auto iter = json.MemberBegin(); iter != json.MemberEnd(); ++iter) {
        ParallelGroupInfo info;
        info.group =JsonUtil::GetStringWithoutKey(iter->name);
        info.globalRanks = JsonUtil::GetVector<std::string>(iter->value, GLOBAL_RANKS);
        info.groupName = JsonUtil::GetString(iter->value, GLOBAL_NAME);
        res.push_back(info);
    }
    return res;
}
}