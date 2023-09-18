/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 */

#include <cmath>
#include "json.hpp"
#include "ServerLog.h"
#include "DataBaseManager.h"
#include "CommunicationMatrixHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;

CommunicationMatrixHandler::CommunicationMatrixHandler()
{
    ServerLog::Info("CommunicationMatrixHandler construct");
}

CommunicationMatrixHandler::~CommunicationMatrixHandler() {}

bool CommunicationMatrixHandler::start_object(std::size_t elements)
{
    currentDepth++;
    Server::ServerLog::Debug("start_object elements:", elements, " depth=", currentDepth, " key=", currentKey);
    return true;
}

bool CommunicationMatrixHandler::boolean(bool val)
{
    currentObject[currentKey] = val;
    Server::ServerLog::Debug("boolean val:", val, " depth=", currentDepth, " key=", currentKey);
    return true;
}

bool CommunicationMatrixHandler::number_integer(number_integer_t val)
{
    currentObject[currentKey] = val;
    Server::ServerLog::Debug("number_integer val:", val, " depth=", currentDepth, " key=", currentKey);
    return true;
}

bool CommunicationMatrixHandler::number_unsigned(number_unsigned_t val)
{
    Server::ServerLog::Debug("number_unsigned val:", val, " depth=", currentDepth, " key=",
                             currentKey);
    currentObject[currentKey] = val;
    return true;
}

bool CommunicationMatrixHandler::number_float(number_float_t val, const string_t &s)
{
    Server::ServerLog::Debug("number_float val:", val, " depth=", currentDepth, " key=", currentKey);
    currentObject[currentKey] = val;
    return true;
}

bool CommunicationMatrixHandler::string(string_t &val)
{
    currentObject[currentKey] = val;
    Server::ServerLog::Debug("string val:", val, " depth=", currentDepth, " key=", currentKey);
    return true;
}

bool CommunicationMatrixHandler::key(string_t &val)
{
    Server::ServerLog::Debug("key currentDepth:", currentDepth, " key=", currentKey, " val=", val);
    currentKey = val;
    if (currentDepth == groupDepth)groupId = val;
    if (currentDepth == stepDepth)iterationId = val;
    if (currentDepth == opNameDepth)tempOpName = val;
    if (currentDepth == ranksDepth)tempRank = val;
    return true;
}

bool CommunicationMatrixHandler::end_object()
{
    auto database = DataBaseManager::Instance().GetClusterDatabase();
    currentDepth--;
    if (currentDepth == ranksDepth) {
        Server::ServerLog::Debug("save Communication matrix:", currentObject);
        CommunicationMatrixInfo matrix = MapToMatrixInfo(currentObject);
        database->InsertCommunicationMatrix(matrix);
        currentObject = nlohmann::json({});
    }
    Server::ServerLog::Debug("end object currentDepth:", currentDepth, " key=", currentKey);
    return true;
}

bool CommunicationMatrixHandler::null()
{
    return true;
}

bool CommunicationMatrixHandler::binary(binary_t &val)
{
    return true;
}

bool CommunicationMatrixHandler::start_array(std::size_t elements)
{
    Server::ServerLog::Debug("start_array depth:", currentDepth, " key=", currentKey);
    return true;
}

bool CommunicationMatrixHandler::end_array()
{
    Server::ServerLog::Debug("end array depth", currentDepth, " key=", currentKey);
    return true;
}

bool CommunicationMatrixHandler::parse_error(std::size_t position, const std::string &last_token,
                                             const nlohmann::detail::exception &ex)
{
    exception = std::string(ex.what());
    Server::ServerLog::Error("Failed parse communication json file. error:", exception);
    return false;
}

CommunicationMatrixInfo CommunicationMatrixHandler::MapToMatrixInfo(const json_t &json)
{
    CommunicationMatrixInfo matrixInfo;
    matrixInfo.groupId = groupId;
    matrixInfo.iterationId = iterationId;
    if (std::strcmp(iterationId.c_str(), "step") == 0) {
        matrixInfo.iterationId = "0";
    }
    int nameIndex = tempOpName.empty() ? 0 : tempOpName.find_last_of('@');
    if (nameIndex > 0) {
        matrixInfo.opName = tempOpName.substr(0, nameIndex);
        matrixInfo.groupName = tempOpName.substr(nameIndex + 1);
    } else {
        matrixInfo.opName = tempOpName;
    }
    int rankIndex = tempRank.empty() ? 0 : tempRank.find_last_of('-');
    if (rankIndex > 0) {
        matrixInfo.srcRank = atof(tempRank.substr(0, rankIndex).c_str());
        matrixInfo.dstRank =  atof(tempRank.substr(rankIndex + 1).c_str());
    } else {
        matrixInfo.srcRank = atof(tempRank.c_str());
    }
    matrixInfo.transportType = json["Transport Type"];
    matrixInfo.transitTime = json["Transit Time(ms)"];
    matrixInfo.transitSize = json["Transit Size(MB)"];
    matrixInfo.bandwidth = json["Bandwidth(GB/s)"];
    return matrixInfo;
}

} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic