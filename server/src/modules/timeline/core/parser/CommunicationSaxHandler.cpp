/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 */

#include "CommunicationSaxHandler.h"
#include <utility>
#include "json.hpp"
#include "ServerLog.h"
#include "DataBaseManager.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;

CommunicationSaxHandler::CommunicationSaxHandler()
{
    ServerLog::Info("CommunicationSaxHandler construct");
}

CommunicationSaxHandler::~CommunicationSaxHandler() {}

bool CommunicationSaxHandler::start_object(std::size_t elements)
{
    currentDepth++;
    Server::ServerLog::Debug("start_object elements:", elements, " depth=", currentDepth, " key=", currentKey);
    return true;
}

bool CommunicationSaxHandler::boolean(bool val)
{
    currentObject[currentKey] = val;
    Server::ServerLog::Debug("boolean val:", val, " depth=", currentDepth, " key=", currentKey);
    return true;
}

bool CommunicationSaxHandler::number_integer(number_integer_t val)
{
    currentObject[currentKey] = val;
    Server::ServerLog::Debug("number_integer val:", val, " depth=", currentDepth, " key=", currentKey);
    return true;
}

bool CommunicationSaxHandler::number_unsigned(number_unsigned_t val)
{
    Server::ServerLog::Debug("number_unsigned val:", val, " depth=", currentDepth, " key=",
                             currentKey);
    if (currentDepth == sizeDistributionDepth) {
        tempInt = val;
    } else {
        currentObject[currentKey] = val;
    }
    return true;
}

bool CommunicationSaxHandler::number_float(number_float_t val, const string_t &s)
{
    Server::ServerLog::Debug("number_float val:", val, " depth=", currentDepth, " key=", currentKey);
    if (currentDepth == sizeDistributionDepth) {
        tempFloat = val;
    } else {
        currentObject[currentKey] = val;
    }
    return true;
}

bool CommunicationSaxHandler::string(string_t &val)
{
    currentObject[currentKey] = val;
    Server::ServerLog::Debug("string val:", val, " depth=", currentDepth, " key=", currentKey);
    return true;
}

bool CommunicationSaxHandler::key(string_t &val)
{
    Server::ServerLog::Debug("key currentDepth:", currentDepth, " key=", currentKey, " val=", val);
    currentKey = val;
    if (currentDepth == stageIdDepth)stageId = val;
    if (currentDepth == stepIdDepth)stepId = val;
    if (currentDepth == tempOpNameDepth)tempOpName = val;
    if (currentDepth == rankIdDepth)rankId = val;
    if (currentDepth == tableFlagDepth)tableFlag = val;
    if (currentDepth == infoDepth && std::strcmp("Communication Bandwidth Info", tableFlag.c_str()) == 0) {
        transportType = val;
        Server::ServerLog::Debug("currentDepth:", currentDepth, " transportType=", val);
    }
    return true;
}

bool CommunicationSaxHandler::end_object()
{
    auto database = DataBaseManager::Instance().GetClusterDatabase();
    currentDepth--;
    if (currentDepth == infoDepth && std::strcmp(tableFlag.c_str(), "Communication Bandwidth Info") == 0) {
        Server::ServerLog::Debug("save Communication Bandwidth Info:", currentObject);
        CommunicationBandWidth bandWidth = MapToBandwidth(currentObject);
        database->InsertBandwidth(bandWidth);
        currentObject = nlohmann::json({});
    }

    if (currentDepth == tableFlagDepth && std::strcmp(tableFlag.c_str(), "Communication Time Info") == 0) {
        Server::ServerLog::Debug("save Communication Time Info:", currentObject);
        CommunicationTimeInfo timeInfo = MapToTimeInfo(currentObject);
        database->InsertTimeInfo(timeInfo);
        currentObject = nlohmann::json({});
    }
    if (currentDepth == infoDepthSeven) {
        Server::ServerLog::Debug("save sizeDistribution :", sizeDistribution);
        currentObject["Size Distribution"] = sizeDistribution;
        sizeDistribution.clear();
    }

    Server::ServerLog::Debug("end object currentDepth:", currentDepth, " key=", currentKey);
    return true;
}

bool CommunicationSaxHandler::null()
{
    return true;
}

bool CommunicationSaxHandler::binary(binary_t &val)
{
    return true;
}

bool CommunicationSaxHandler::start_array(std::size_t elements)
{
    Server::ServerLog::Debug("start_array depth:", currentDepth, " key=", currentKey);
    return true;
}

bool CommunicationSaxHandler::end_array()
{
    Server::ServerLog::Debug("end array depth", currentDepth, " key=", currentKey);
    if (currentDepth == sizeDistributionDepth) {
        sizeDistribution[currentKey] = {tempInt, tempFloat};
    }
    return true;
}

bool CommunicationSaxHandler::parse_error(std::size_t position, const std::string &last_token,
                                          const nlohmann::detail::exception &ex)
{
    exception = std::string(ex.what());
    Server::ServerLog::Error("Failed parse communication json file. error:", exception);
    return false;
}

CommunicationBandWidth CommunicationSaxHandler::MapToBandwidth(const json_t &json)
{
    CommunicationBandWidth bandWidth;
    bandWidth.stageId = stageId;
    bandWidth.iterationId = stepId.length() > stepSubLen ? stepId.substr(stepSubLen) : stepId;
    bandWidth.rankId = rankId;
    int index = tempOpName.empty() ? 0 : tempOpName.find_last_of('@');
    if (index > 0) {
        bandWidth.opName = tempOpName.substr(0, index);
        bandWidth.opSuffix = tempOpName.substr(index + 1);
    } else {
        bandWidth.opName = tempOpName;
    }
    bandWidth.transportType = transportType;
    bandWidth.bandwidthSize = json["Bandwidth(GB/s)"];
    bandWidth.sizeDistribution = to_string(json["Size Distribution"]);
    bandWidth.transitSize = json["Transit Size(MB)"];
    bandWidth.transitTime = json["Transit Time(ms)"];
    return bandWidth;
}

CommunicationTimeInfo CommunicationSaxHandler::MapToTimeInfo(const json_t &json)
{
    CommunicationTimeInfo timeInfo;
    timeInfo.rankId = rankId;
    // 去掉step前缀
    timeInfo.iterationId = stepId.length() > stepSubLen ? stepId.substr(stepSubLen) : stepId;
    timeInfo.stageId = stageId;
    int index = tempOpName.empty() ? 0 : tempOpName.find_last_of('@');
    if (index > 0) {
        timeInfo.opName = tempOpName.substr(0, index);
        timeInfo.opSuffix = tempOpName.substr(index + 1);
    } else {
        timeInfo.opName = tempOpName;
    }
    timeInfo.elapseTime = json["Elapse Time(ms)"];
    timeInfo.idleTime = json["Idle Time(ms)"];
    timeInfo.synchronizationTimeRatio = json["Synchronization Time Ratio"];
    timeInfo.synchronizationTime = json["Synchronization Time(ms)"];
    timeInfo.transitTime = json["Transit Time(ms)"];
    timeInfo.waitTimeRatio = json["Wait Time Ratio"];
    timeInfo.waitTime = json["Wait Time(ms)"];
    return timeInfo;
}
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic