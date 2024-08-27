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

CommunicationRapidSaxHandler::CommunicationRapidSaxHandler()
{
    currentObject.SetObject();
}

CommunicationRapidSaxHandler::~CommunicationRapidSaxHandler() {}

bool CommunicationRapidSaxHandler::Null()
{
    return true;
}

bool CommunicationRapidSaxHandler::Bool(bool b)
{
    rapidjson::Value tempKey(currentKey.c_str(), currentObject.GetAllocator());
    currentObject.AddMember(tempKey, b, currentObject.GetAllocator());
    return true;
}

bool CommunicationRapidSaxHandler::Int(int i)
{
    rapidjson::Value tempKey(currentKey.c_str(), currentObject.GetAllocator());
    currentObject.AddMember(tempKey, i, currentObject.GetAllocator());
    return true;
}

bool CommunicationRapidSaxHandler::Uint(unsigned int u)
{
    if (currentDepth == sizeDistributionDepth) {
        tempInt = u;
    } else {
        rapidjson::Value tempKey(currentKey.c_str(), currentObject.GetAllocator());
        currentObject.AddMember(tempKey, u, currentObject.GetAllocator());
    }
    return true;
}

bool CommunicationRapidSaxHandler::Int64(int64_t i)
{
    rapidjson::Value tempKey(currentKey.c_str(), currentObject.GetAllocator());
    currentObject.AddMember(tempKey, i, currentObject.GetAllocator());
    return true;
}

bool CommunicationRapidSaxHandler::Uint64(uint64_t u)
{
    rapidjson::Value tempKey(currentKey.c_str(), currentObject.GetAllocator());
    currentObject.AddMember(tempKey, u, currentObject.GetAllocator());
    return true;
}

bool CommunicationRapidSaxHandler::Double(double d)
{
    if (currentDepth == sizeDistributionDepth) {
        tempTransitSize = d;
    } else {
        rapidjson::Value tempKey(currentKey.c_str(), currentObject.GetAllocator());
        currentObject.AddMember(tempKey, d, currentObject.GetAllocator());
    }
    return true;
}

bool CommunicationRapidSaxHandler::RawNumber(const char *str, SizeType len, bool copy)
{
    if (currentDepth != sizeDistributionDepth) {
        return BaseReaderHandler::RawNumber(str, len, copy);
    }

    const std::string numberStr(str);
    if (StringUtil::Contains(numberStr, ".")) {
        tempTransitSize = std::strtod(str, nullptr);
    } else {
        tempInt = std::strtol(str, nullptr, INT_TEN);
    }
    return true;
}

bool CommunicationRapidSaxHandler::String(const char *str, rapidjson::SizeType length, bool copy)
{
    rapidjson::Value tempKey(currentKey.c_str(), currentObject.GetAllocator());
    rapidjson::Value val(str, currentObject.GetAllocator());
    currentObject.AddMember(tempKey, val, currentObject.GetAllocator());
    return true;
}

bool CommunicationRapidSaxHandler::StartObject()
{
    currentDepth++;
    if (currentDepth == infoDepthSeven) {
        sizeDistribution = rapidjson::Value(rapidjson::kObjectType);
    }
    return true;
}

bool CommunicationRapidSaxHandler::Key(const char *str, rapidjson::SizeType length, bool copy)
{
    currentKey = str;
    if (currentDepth == stageIdDepth) { stageId = str; }
    if (currentDepth == stepIdDepth) { stepId = str; }
    if (currentDepth == tempOpNameDepth) { tempOpName = str; }
    if (currentDepth == rankIdDepth) { rankId = str; }
    if (currentDepth == tableFlagDepth) { tableFlag = str; }
    if (currentDepth == infoDepth && std::strcmp("Communication Bandwidth Info", tableFlag.c_str()) == 0) {
        transportType = str;
    }
    return true;
}

bool CommunicationRapidSaxHandler::EndObject(rapidjson::SizeType memberCount)
{
    if (ParserStatusManager::Instance().GetClusterParserStatus() != ParserStatus::RUNNING) {
        return false;
    }
    // 获取所有的groupId映射关系
    auto databaseRead = dynamic_cast<TextClusterDatabase*>(DataBaseManager::Instance().GetReadClusterDatabase());
    if (databaseRead == nullptr) {
        ServerLog::Error("Can't get cluster database for read when parse communication data.");
        return false;
    }
    if (groupIdsMap.empty()) {
        groupIdsMap = databaseRead->GetAllGroupMap();
    }
    auto database = dynamic_cast<TextClusterDatabase*>(DataBaseManager::Instance().GetWriteClusterDatabase());
    if (database == nullptr) {
        ServerLog::Error("Can't get cluster database for write when parse communication data.");
        return false;
    }
    currentDepth--;
    if (currentDepth == infoDepth && std::strcmp(tableFlag.c_str(), "Communication Bandwidth Info") == 0) {
        CommunicationBandWidth bandWidth = MapToBandwidth(currentObject);
        database->InsertBandwidth(bandWidth);
        currentObject.RemoveAllMembers();
    }

    if (currentDepth == tableFlagDepth && std::strcmp(tableFlag.c_str(), "Communication Time Info") == 0) {
        CommunicationTimeInfo timeInfo = MapToTimeInfo(currentObject);
        database->InsertTimeInfo(timeInfo);
        currentObject.RemoveAllMembers();
    }
    if (currentDepth == infoDepthSeven) {
        currentObject.AddMember("Size Distribution", sizeDistribution, currentObject.GetAllocator());
    }
    return true;
}

bool CommunicationRapidSaxHandler::StartArray()
{
    return true;
}

bool CommunicationRapidSaxHandler::EndArray(rapidjson::SizeType elementCount)
{
    if (currentDepth == sizeDistributionDepth) {
        rapidjson::Value tempKey(currentKey.c_str(), currentObject.GetAllocator());
        rapidjson::Value array(rapidjson::kArrayType);
        rapidjson::Value tempIntVal(tempInt);
        rapidjson::Value tempTransitSizeVal(tempTransitSize);
        array.PushBack(tempIntVal, currentObject.GetAllocator());
        array.PushBack(tempTransitSizeVal, currentObject.GetAllocator());
        sizeDistribution.AddMember(tempKey, array, currentObject.GetAllocator());
    }
    return true;
}

CommunicationBandWidth CommunicationRapidSaxHandler::MapToBandwidth(const rapidjson::Document &json)
{
    CommunicationBandWidth bandWidth;
    bandWidth.stageId = std::to_string(groupIdsMap[stageId]);
    bandWidth.iterationId = stepId.length() > stepSubLen ? stepId.substr(stepSubLen) : stepId;
    if (std::strcmp(stepId.c_str(), "step") == 0) {
        bandWidth.iterationId = "0";
    }
    bandWidth.rankId = rankId;
    size_t index = tempOpName.empty() ? 0 : tempOpName.find_last_of('@');
    if (index != std::string::npos) {
        bandWidth.opName = tempOpName.substr(0, index);
        bandWidth.opSuffix = tempOpName.substr(index + 1);
    } else {
        bandWidth.opName = tempOpName;
    }
    bandWidth.transportType = transportType;
    bandWidth.bandwidthSize = JsonUtil::GetDouble(json, "Bandwidth(GB/s)");
    bandWidth.sizeDistribution = JsonUtil::GetDumpString(json, "Size Distribution");
    bandWidth.transitSize = JsonUtil::GetDouble(json, "Transit Size(MB)");
    bandWidth.transitTime = JsonUtil::GetDouble(json, "Transit Time(ms)");
    return bandWidth;
}

CommunicationTimeInfo CommunicationRapidSaxHandler::MapToTimeInfo(const rapidjson::Document &json)
{
    CommunicationTimeInfo timeInfo;
    timeInfo.rankId = rankId;
    // 去掉step前缀
    timeInfo.iterationId = stepId.length() > stepSubLen ? stepId.substr(stepSubLen) : stepId;
    if (std::strcmp(stepId.c_str(), "step") == 0) {
        timeInfo.iterationId = "0";
    }
    timeInfo.stageId = std::to_string(groupIdsMap[stageId]);
    size_t index = tempOpName.empty() ? 0 : tempOpName.find_last_of('@');
    if (index != std::string::npos) {
        timeInfo.opName = tempOpName.substr(0, index);
        timeInfo.opSuffix = tempOpName.substr(index + 1);
    } else {
        timeInfo.opName = tempOpName;
    }
    timeInfo.startTime = NumberUtil::TimestampUsToNs(JsonUtil::GetString(json, "Start Timestamp(us)"));
    timeInfo.elapseTime = JsonUtil::GetDouble(json, "Elapse Time(ms)");
    timeInfo.idleTime = JsonUtil::GetDouble(json, "Idle Time(ms)");
    timeInfo.synchronizationTimeRatio = JsonUtil::GetDouble(json, "Synchronization Time Ratio");
    timeInfo.synchronizationTime = JsonUtil::GetDouble(json, "Synchronization Time(ms)");
    timeInfo.transitTime = JsonUtil::GetDouble(json, "Transit Time(ms)");
    timeInfo.waitTimeRatio = JsonUtil::GetDouble(json, "Wait Time Ratio");
    timeInfo.waitTime = JsonUtil::GetDouble(json, "Wait Time(ms)");
    return timeInfo;
}
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic