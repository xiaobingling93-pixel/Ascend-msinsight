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
    CommunicationRapidSaxHandler();
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
    CommunicationBandWidth MapToBandwidth(const rapidjson::Document &json);
    CommunicationTimeInfo MapToTimeInfo(const rapidjson::Document &json);
private:
    double tempTransitSize = 0;
    int tempInt = 0;
    rapidjson::Document currentObject;
    rapidjson::Value sizeDistribution;
    std::string currentKey;
    std::string rankId;
    std::string stepId;
    std::string stageId;
    std::string tempOpName;
    std::string transportType;
    std::string tableFlag;
    std::string selectedPath;
    int currentDepth = 0;
    int stageIdDepth = 1;
    int stepIdDepth = 2;
    int tempOpNameDepth = 3;
    int rankIdDepth = 4;
    int stepSubLen = 4;
    int tableFlagDepth = 5;
    int infoDepth = 6;
    int infoDepthSeven = 7;
    int sizeDistributionDepth = 8;
    std::string exception;
    std::unordered_map<std::string, int64_t> groupIdsMap;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_COMMUNICATIONRAPIDSAXHANDLER_H
