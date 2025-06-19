/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_COMMUNICATIONRAPIDSAXHANDLER_H
#define PROFILER_SERVER_COMMUNICATIONRAPIDSAXHANDLER_H

#include <iostream>
#include "encodings.h"
#include "reader.h"
#include "document.h"
#include "VirtualClusterDatabase.h"
#include "GlobalDefs.h"
#include "DataBaseManager.h"

namespace Dic {
namespace Module {
namespace Timeline {

class CommunicationRapidSaxHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>,
        CommunicationRapidSaxHandler> {
public:
    explicit CommunicationRapidSaxHandler(std::shared_ptr<TextClusterDatabase> database, const std::string &uniqueKey);
    ~CommunicationRapidSaxHandler();
    bool Null();
    bool Bool(bool b);
    bool Int(int i);
    bool Uint(unsigned u);
    bool Int64(int64_t i);
    bool Uint64(uint64_t u) ;
    bool Double(double d) ;
    bool RawNumber(const Ch* str, SizeType len, bool copy);
    bool String(const char* str, rapidjson::SizeType length, bool copy) ;
    bool StartObject();
    bool Key(const char* str, rapidjson::SizeType length, bool copy) ;
    bool EndObject(rapidjson::SizeType memberCount);
    bool StartArray();
    bool EndArray(rapidjson::SizeType elementCount);
    void GetBandwidth();
    void GetTimeInfo();
    std::string GetIndexByStage(const std::string &stage);
private:
    std::string currentKey;
    std::string rankId;
    std::string stepId;
    std::string stageId;
    std::string tempOpName;
    std::string transportType;
    std::string tableFlag;
    std::string selectedPath;
    uint32_t currentDepth = 0;
    uint32_t stageIdDepth = 1;
    uint32_t stepIdDepth = 2;
    uint32_t tempOpNameDepth = 3;
    uint32_t rankIdDepth = 4;
    uint32_t stepSubLen = 4;
    uint32_t tableFlagDepth = 5;
    uint32_t infoDepth = 6;
    uint32_t infoDepthSeven = 7;
    uint32_t sizeDistributionDepth = 8;
    std::string exception;
    std::unordered_map<std::string, int64_t> groupIdsMap;
    std::shared_ptr<TextClusterDatabase> database;
    std::string uniqueKey;

    CommunicationBandWidth bandwidth;
    CommunicationTimeInfo timeInfo;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_COMMUNICATIONRAPIDSAXHANDLER_H
