/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */

#include "JsonUtil.h"
#include "ParserStatusManager.h"
#include "JsonClusterDatabase.h"
#include "CommunicationMatrixRapidHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
CommunicationMatrixRapidHandler::CommunicationMatrixRapidHandler()
{
    currentObject.SetObject();
}

CommunicationMatrixRapidHandler::~CommunicationMatrixRapidHandler() {}

bool CommunicationMatrixRapidHandler::Null()
{
    return true;
}

bool CommunicationMatrixRapidHandler::Bool(bool bl)
{
    rapidjson::Value tempKey(currentKey.c_str(), currentObject.GetAllocator());
    currentObject.AddMember(tempKey, bl, currentObject.GetAllocator());
    return true;
}

bool CommunicationMatrixRapidHandler::Int(int i)
{
    rapidjson::Value tempKey(currentKey.c_str(), currentObject.GetAllocator());
    currentObject.AddMember(tempKey, i, currentObject.GetAllocator());
    return true;
}

bool CommunicationMatrixRapidHandler::Uint(unsigned int uint)
{
    rapidjson::Value tempKey(currentKey.c_str(), currentObject.GetAllocator());
    currentObject.AddMember(tempKey, uint, currentObject.GetAllocator());
    return true;
}

bool CommunicationMatrixRapidHandler::Int64(int64_t i)
{
    rapidjson::Value tempKey(currentKey.c_str(), currentObject.GetAllocator());
    currentObject.AddMember(tempKey, i, currentObject.GetAllocator());
    return true;
}

bool CommunicationMatrixRapidHandler::Uint64(uint64_t u)
{
    rapidjson::Value tempKey(currentKey.c_str(), currentObject.GetAllocator());
    currentObject.AddMember(tempKey, u, currentObject.GetAllocator());
    return true;
}

bool CommunicationMatrixRapidHandler::Double(double doubleVal)
{
    rapidjson::Value tempKey(currentKey.c_str(), currentObject.GetAllocator());
    currentObject.AddMember(tempKey, doubleVal, currentObject.GetAllocator());
    return true;
}

bool CommunicationMatrixRapidHandler::String(const char *str, rapidjson::SizeType length, bool copy)
{
    rapidjson::Value tempKey(currentKey.c_str(), currentObject.GetAllocator());
    rapidjson::Value val(str, currentObject.GetAllocator());
    currentObject.AddMember(tempKey, val, currentObject.GetAllocator());
    return true;
}

bool CommunicationMatrixRapidHandler::StartObject()
{
    currentDepth++;
    return true;
}

bool CommunicationMatrixRapidHandler::Key(const char *str, rapidjson::SizeType length, bool copy)
{
    currentKey = str;
    if (currentDepth == groupDepth) { groupId = str; }
    if (currentDepth == stepDepth) { iterationId = str; }
    if (currentDepth == opNameDepth) { tempOpName = str; }
    if (currentDepth == ranksDepth) { tempRank = str; }
    return true;
}

bool CommunicationMatrixRapidHandler::EndObject(rapidjson::SizeType memberCount)
{
    if (ParserStatusManager::Instance().GetClusterParserStatus() != ParserStatus::RUNNING) {
        return false;
    }
    auto database = dynamic_cast<JsonClusterDatabase*>(DataBaseManager::Instance().GetWriteClusterDatabase());
    currentDepth--;
    if (currentDepth == ranksDepth) {
        CommunicationMatrixInfo matrix = MapToMatrixInfo(currentObject);
        database->InsertCommunicationMatrix(matrix);
        currentObject.RemoveAllMembers();
    }
    if (currentDepth == 0) {
        database->InsertGroupId(groupIdsMap);
    }
    return true;
}

bool CommunicationMatrixRapidHandler::StartArray()
{
    return true;
}

bool CommunicationMatrixRapidHandler::EndArray(rapidjson::SizeType elementCount)
{
    return true;
}

CommunicationMatrixInfo CommunicationMatrixRapidHandler::MapToMatrixInfo(const rapidjson::Document &json)
{
    CommunicationMatrixInfo matrixInfo;
    auto it = groupIdsMap.insert(std::make_pair(groupId, groupIdNumber));
    if (it.second) {
        groupIdNumber++;
    }
    matrixInfo.groupId = std::to_string(groupIdsMap[groupId]);
    matrixInfo.iterationId = iterationId;
    matrixInfo.iterationId = iterationId.length() > stepSubLen ? iterationId.substr(stepSubLen) : iterationId;
    if (std::strcmp(iterationId.c_str(), "step") == 0) {
        matrixInfo.iterationId = "0";
    }
    int nameIndex = tempOpName.empty() ? 0 : tempOpName.find_last_of('@');
    if (nameIndex > 0) {
        matrixInfo.sortOp = tempOpName.substr(0, nameIndex);
        matrixInfo.groupName = tempOpName.substr(nameIndex + 1);
    } else {
        matrixInfo.sortOp = tempOpName;
    }
    int rankIndex = tempRank.empty() ? 0 : tempRank.find_last_of('-');
    if (rankIndex > 0) {
        matrixInfo.srcRank = atof(tempRank.substr(0, rankIndex).c_str());
        matrixInfo.dstRank =  atof(tempRank.substr(rankIndex + 1).c_str());
    } else {
        matrixInfo.srcRank = atof(tempRank.c_str());
    }
    matrixInfo.transportType = JsonUtil::GetDumpString(json, "Transport Type");
    matrixInfo.transitTime = JsonUtil::GetDouble(json, "Transit Time(ms)");
    matrixInfo.transitSize = JsonUtil::GetDouble(json, "Transit Size(MB)");
    matrixInfo.bandwidth = JsonUtil::GetDouble(json, "Bandwidth(GB/s)");
    matrixInfo.opName = JsonUtil::GetString(json, "Op Name");
    return matrixInfo;
}

} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic