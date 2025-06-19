/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */

#include "pch.h"
#include "ParserStatusManager.h"
#include "TextClusterDatabase.h"
#include "CommunicationRapidSaxHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {

CommunicationRapidSaxHandler::CommunicationRapidSaxHandler(std::shared_ptr<TextClusterDatabase> database,
    const std::string &uniqueKey) : database(database), uniqueKey(uniqueKey) {}

CommunicationRapidSaxHandler::~CommunicationRapidSaxHandler() {}

bool CommunicationRapidSaxHandler::Null()
{
    return true;
}

bool CommunicationRapidSaxHandler::Bool(bool b)
{
    return true;
}

bool CommunicationRapidSaxHandler::Int(int i)
{
    return true;
}

bool CommunicationRapidSaxHandler::Uint(unsigned int u)
{
    return true;
}

bool CommunicationRapidSaxHandler::Int64(int64_t i)
{
    return true;
}

bool CommunicationRapidSaxHandler::Uint64(uint64_t u)
{
    return true;
}

bool CommunicationRapidSaxHandler::Double(double d)
{
    return true;
}

bool CommunicationRapidSaxHandler::RawNumber(const char *str, SizeType len, bool copy)
{
    if (str == nullptr) {
        ServerLog::Error("Parsing communication.json encounters nullptr.");
        return false;
    }
    std::string s(str);
    if (currentDepth == sizeDistributionDepth) {
        bandwidth.sizeDistribution += s + ",";
        return true;
    }
    if (currentDepth == tableFlagDepth + 1 && tableFlag == "Communication Time Info") {
        if (currentKey == "Start Timestamp(us)") {
            int64_t tempStartTime = NumberUtil::TimestampUsToNs(s);
            timeInfo.startTime = NumberUtil::Int64ToUint64(tempStartTime);
        }
        if (currentKey == "Elapse Time(ms)") {
            timeInfo.elapseTime = NumberUtil::StringToDouble(s);
        }
        if (currentKey == "Idle Time(ms)") {
            timeInfo.idleTime = NumberUtil::StringToDouble(s);
        }
        if (currentKey == "Synchronization Time Ratio") {
            timeInfo.synchronizationTimeRatio = NumberUtil::StringToDouble(s);
        }
        if (currentKey == "Synchronization Time(ms)") {
            timeInfo.synchronizationTime = NumberUtil::StringToDouble(s);
        }
        if (currentKey == "Transit Time(ms)") {
            timeInfo.transitTime = NumberUtil::StringToDouble(s);
        }
        if (currentKey == "Wait Time Ratio") {
            timeInfo.waitTimeRatio = NumberUtil::StringToDouble(s);
        }
        if (currentKey == "Wait Time(ms)") {
            timeInfo.waitTime = NumberUtil::StringToDouble(s);
        }
    }
    if (currentDepth == infoDepth + 1 && tableFlag == "Communication Bandwidth Info") {
        if (currentKey == "Bandwidth(GB/s)") {
            bandwidth.bandwidthSize = NumberUtil::StringToDouble(s);
        }
        if (currentKey == "Transit Size(MB)") {
            bandwidth.transitSize = NumberUtil::StringToDouble(s);
        }
        if (currentKey == "Transit Time(ms)") {
            bandwidth.transitTime = NumberUtil::StringToDouble(s);
        }
    }
    return true;
}

bool CommunicationRapidSaxHandler::String(const char *str, rapidjson::SizeType length, bool copy)
{
    return true;
}

bool CommunicationRapidSaxHandler::StartObject()
{
    currentDepth++;
    if (currentDepth == sizeDistributionDepth) {
        bandwidth.sizeDistribution = "{";
    }
    return true;
}

bool CommunicationRapidSaxHandler::Key(const char *str, rapidjson::SizeType length, bool copy)
{
    currentKey = str;
    if (currentDepth == stageIdDepth) {
        if (currentKey == "p2p") {
            stageId = str;
        } else {
            std::vector<std::string> rankList = StringUtil::SplitStringWithParenthesesByComma(str);
            stageId = StringUtil::JoinNumberStrWithParenthesesByOrder(rankList);
        }
    }
    if (currentDepth == stepIdDepth) { stepId = str; }
    if (currentDepth == tempOpNameDepth) { tempOpName = str; }
    if (currentDepth == rankIdDepth) { rankId = str; }
    if (currentDepth == tableFlagDepth) { tableFlag = str; }
    if (currentDepth == infoDepth && std::strcmp("Communication Bandwidth Info", tableFlag.c_str()) == 0) {
        transportType = str;
    }
    if (currentDepth == sizeDistributionDepth) {
        bandwidth.sizeDistribution += "\"" + currentKey + "\"";
    }
    return true;
}

bool CommunicationRapidSaxHandler::EndObject(rapidjson::SizeType memberCount)
{
    if (ParserStatusManager::Instance().IsClusterParserFinalState(uniqueKey)) {
        return false;
    }
    // 获取所有的groupId映射关系
    if (database == nullptr) {
        ServerLog::Error("Can't get cluster database for read when parse communication data.");
        return false;
    }
    if (groupIdsMap.empty()) {
        groupIdsMap = database->GetAllGroupMap();
    }
    if (database == nullptr) {
        ServerLog::Error("Can't get cluster database for write when parse communication data.");
        return false;
    }
    currentDepth--;
    if (currentDepth == infoDepth && std::strcmp(tableFlag.c_str(), "Communication Bandwidth Info") == 0) {
        GetBandwidth();
        database->InsertBandwidth(bandwidth);
        bandwidth = CommunicationBandWidth{};
    }

    if (currentDepth == tableFlagDepth && std::strcmp(tableFlag.c_str(), "Communication Time Info") == 0) {
        GetTimeInfo();
        database->InsertTimeInfo(timeInfo);
        timeInfo = CommunicationTimeInfo{};
    }
    if (currentDepth == infoDepthSeven) {
        if (StringUtil::EndWith(bandwidth.sizeDistribution, ",")) {
            bandwidth.sizeDistribution.resize(bandwidth.sizeDistribution.size() - 1);
        }
        bandwidth.sizeDistribution += "}";
    }
    return true;
}

bool CommunicationRapidSaxHandler::StartArray()
{
    bandwidth.sizeDistribution += ":[";
    return true;
}

bool CommunicationRapidSaxHandler::EndArray(rapidjson::SizeType elementCount)
{
    if (currentDepth == sizeDistributionDepth) {
        if (StringUtil::EndWith(bandwidth.sizeDistribution, ",")) {
            bandwidth.sizeDistribution.resize(bandwidth.sizeDistribution.size() - 1);
        }
        bandwidth.sizeDistribution += "],";
    }
    return true;
}

std::string CommunicationRapidSaxHandler::GetIndexByStage(const std::string &stage)
{
    if (groupIdsMap.count(stageId) == 0) {
        uint64_t curIndex = 0;
        CommGroupParallelInfo info;
        info.rankSetStr = stageId;
        info.type = "collective";
        if (database->InsertGroupInfoReturnIndex(info, curIndex)) {
            groupIdsMap.insert({stageId, curIndex});
        }
    }
    return std::to_string(groupIdsMap[stageId]);
}

void CommunicationRapidSaxHandler::GetBandwidth()
{
    bandwidth.stageId = GetIndexByStage(stageId);
    bandwidth.iterationId = stepId.length() > stepSubLen ? stepId.substr(stepSubLen) : stepId;
    if (std::strcmp(stepId.c_str(), "step") == 0) {
        bandwidth.iterationId = "0";
    }
    bandwidth.rankId = rankId;
    size_t index = tempOpName.find_last_of('@');
    if (index != std::string::npos) {
        bandwidth.opName = tempOpName.substr(0, index);
        bandwidth.opSuffix = tempOpName.substr(index + 1);
    } else {
        bandwidth.opName = tempOpName;
    }
    bandwidth.transportType = transportType;
}

void CommunicationRapidSaxHandler::GetTimeInfo()
{
    timeInfo.rankId = rankId;
    // 去掉step前缀
    timeInfo.iterationId = stepId.length() > stepSubLen ? stepId.substr(stepSubLen) : stepId;
    if (std::strcmp(stepId.c_str(), "step") == 0) {
        timeInfo.iterationId = "0";
    }
    timeInfo.stageId = GetIndexByStage(stageId);
    size_t index = tempOpName.find_last_of('@');
    if (index != std::string::npos) {
        timeInfo.opName = tempOpName.substr(0, index);
        timeInfo.opSuffix = tempOpName.substr(index + 1);
    } else {
        timeInfo.opName = tempOpName;
    }
}
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic