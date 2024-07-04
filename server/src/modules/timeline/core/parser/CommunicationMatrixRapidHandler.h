/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_COMMUNICATIONMATRIXRAPIDHANDLER_H
#define PROFILER_SERVER_COMMUNICATIONMATRIXRAPIDHANDLER_H

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
class CommunicationMatrixRapidHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>,
        CommunicationMatrixRapidHandler> {
public:
    CommunicationMatrixRapidHandler();
    ~CommunicationMatrixRapidHandler();
    bool Null();
    bool Bool(bool bl);
    bool Int(int i);
    bool Uint(unsigned uint);
    bool Int64(int64_t i);
    bool Uint64(uint64_t u) ;
    bool Double(double doubleVal) ;
    bool String(const char* val, rapidjson::SizeType length, bool copy) ;
    bool StartObject();
    bool Key(const char* keyStr, rapidjson::SizeType length, bool copy) ;
    bool EndObject(rapidjson::SizeType memberCount);
    bool StartArray();
    bool EndArray(rapidjson::SizeType elementCount);
    CommunicationMatrixInfo MapToMatrixInfo(const rapidjson::Document &json);

private:
    std::string exception;
    int currentDepth = 0;
    rapidjson::Document currentObject;
    std::string currentKey;
    std::string groupId;
    std::string iterationId;
    std::string tempOpName;
    std::string tempRank;
    int64_t groupIdNumber = 1;
    std::unordered_map<std::string, int64_t> groupIdsMap;
    int groupDepth = 1;
    int stepDepth = 2;
    int opNameDepth = 3;
    int ranksDepth = 4;
    int stepSubLen = 4;
};

} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_COMMUNICATIONMATRIXRAPIDHANDLER_H
