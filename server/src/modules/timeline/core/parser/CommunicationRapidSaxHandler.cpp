/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "ServerLog.h"
#include "JsonUtil.h"
#include "ParserStatusManager.h"
#include "CommunicationRapidSaxHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;

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
    Server::ServerLog::Debug("boolean val:", b, " depth=", currentDepth, " key=", currentKey);
    return true;
}

bool CommunicationRapidSaxHandler::Int(int i)
{
    rapidjson::Value tempKey(currentKey.c_str(), currentObject.GetAllocator());
    currentObject.AddMember(tempKey, i, currentObject.GetAllocator());
    Server::ServerLog::Debug("Int val:", i, " depth=", currentDepth, " key=", currentKey);
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
    Server::ServerLog::Debug("Uint val:", u, " depth=", currentDepth, " key=", currentKey);
    return true;
}

bool CommunicationRapidSaxHandler::Int64(int64_t i)
{
    rapidjson::Value tempKey(currentKey.c_str(), currentObject.GetAllocator());
    currentObject.AddMember(tempKey, i, currentObject.GetAllocator());
    Server::ServerLog::Debug("Int64 val:", i, " depth=", currentDepth, " key=", currentKey);
    return true;
}

bool CommunicationRapidSaxHandler::Uint64(uint64_t u)
{
    rapidjson::Value tempKey(currentKey.c_str(), currentObject.GetAllocator());
    currentObject.AddMember(tempKey, u, currentObject.GetAllocator());
    Server::ServerLog::Debug("Uint64 val:", u, " depth=", currentDepth, " key=", currentKey);
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
    Server::ServerLog::Debug("Double val:", d, " depth=", currentDepth, " key=", currentKey);
    return true;
}

bool CommunicationRapidSaxHandler::String(const char *str, rapidjson::SizeType length, bool copy)
{
    rapidjson::Value tempKey(currentKey.c_str(), currentObject.GetAllocator());
    rapidjson::Value val(str, currentObject.GetAllocator());
    currentObject.AddMember(tempKey, val, currentObject.GetAllocator());
    Server::ServerLog::Debug("string val:", str, " depth=", currentDepth, " key=", currentKey);
    return true;
}

bool CommunicationRapidSaxHandler::StartObject()
{
    currentDepth++;
    Server::ServerLog::Debug("start_object elements:", " depth=", currentDepth, " key=", currentKey);
    return true;
}

bool CommunicationRapidSaxHandler::Key(const char *str, rapidjson::SizeType length, bool copy)
{
    Server::ServerLog::Debug("key currentDepth:", currentDepth, " key=", currentKey, " val=", str);
    currentKey = str;
    if (currentDepth == stageIdDepth) { stageId = str; }
    if (currentDepth == stepIdDepth) { stepId = str; }
    if (currentDepth == tempOpNameDepth) { tempOpName = str; }
    if (currentDepth == rankIdDepth) { rankId = str; }
    if (currentDepth == tableFlagDepth) { tableFlag = str; }
    if (currentDepth == infoDepth && std::strcmp("Communication Bandwidth Info", tableFlag.c_str()) == 0) {
        transportType = str;
        Server::ServerLog::Debug("currentDepth:", currentDepth, " transportType=", str);
    }
    return true;
}

bool CommunicationRapidSaxHandler::EndObject(rapidjson::SizeType memberCount)
{
    if (ParserStatusManager::Instance().GetClusterParserStatus() != ParserStatus::RUNNING) {
        return false;
    }
    auto database = DataBaseManager::Instance().GetClusterDatabase();
    currentDepth--;
    if (currentDepth == infoDepth && std::strcmp(tableFlag.c_str(), "Communication Bandwidth Info") == 0) {
        Server::ServerLog::Debug("save Communication Bandwidth Info:", memberCount);
        CommunicationBandWidth bandWidth = MapToBandwidth(currentObject);
        database->InsertBandwidth(bandWidth);
        currentObject.RemoveAllMembers();
    }

    if (currentDepth == tableFlagDepth && std::strcmp(tableFlag.c_str(), "Communication Time Info") == 0) {
        Server::ServerLog::Debug("save Communication Time Info:");
        CommunicationTimeInfo timeInfo = MapToTimeInfo(currentObject);
        database->InsertTimeInfo(timeInfo);
        currentObject.RemoveAllMembers();
    }
    if (currentDepth == infoDepthSeven) {
        Server::ServerLog::Debug("save sizeDistribution :");
        currentObject.AddMember("Size Distribution", sizeDistribution, currentObject.GetAllocator());
    }

    Server::ServerLog::Debug("end object currentDepth:", currentDepth, " key=", currentKey);
    return true;
}

bool CommunicationRapidSaxHandler::StartArray()
{
    Server::ServerLog::Debug("start_array depth:", currentDepth, " key=", currentKey);
    return true;
}

bool CommunicationRapidSaxHandler::EndArray(rapidjson::SizeType elementCount)
{
    Server::ServerLog::Debug("end array depth", currentDepth, " key=", currentKey);
    if (currentDepth == sizeDistributionDepth) {
        sizeDistribution = rapidjson::Value(rapidjson::kObjectType);
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
    bandWidth.stageId = stageId;
    bandWidth.iterationId = stepId.length() > stepSubLen ? stepId.substr(stepSubLen) : stepId;
    if (std::strcmp(stepId.c_str(), "step") == 0) {
        bandWidth.iterationId = "0";
    }
    bandWidth.rankId = rankId;
    int index = tempOpName.empty() ? 0 : tempOpName.find_last_of('@');
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
    timeInfo.stageId = stageId;
    int index = tempOpName.empty() ? 0 : tempOpName.find_last_of('@');
    if (index != std::string::npos) {
        timeInfo.opName = tempOpName.substr(0, index);
        timeInfo.opSuffix = tempOpName.substr(index + 1);
    } else {
        timeInfo.opName = tempOpName;
    }
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