/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: defines declaration
 */

#ifndef DATA_INSIGHT_CORE_DEFS_H
#define DATA_INSIGHT_CORE_DEFS_H

#include "rapidjson.h"
#include "document.h"
#include "writer.h"
#include "stringbuffer.h"

namespace Dic {
using json_t = rapidjson::Value;
using document_t = rapidjson::Document;
using namespace rapidjson;

enum class ParserType {
    DB = 0,
    BIN = 1,
    JSON = 2,
    IPYNB = 3,
    OTHER = 20
};

enum class ProjectTypeEnum {
    DB = 0,
    BIN = 1,
    IPYNB = 2,
    CLUSTER = 3,
    SIMULATION = 4,
    TRACE = 5,
};

static inline ParserType coverProjectTypeToParserType(ProjectTypeEnum projectTypeEnum)
{
    switch (projectTypeEnum) {
        case ProjectTypeEnum::DB:
            return ParserType::DB;
        case ProjectTypeEnum::BIN:
            return ParserType::BIN;
        case ProjectTypeEnum::IPYNB:
            return ParserType::IPYNB;
        default:
            return ParserType::JSON;
    }
}

static inline bool isCoverProjectType(ProjectTypeEnum projectTypeEnum)
{
    return !(projectTypeEnum == ProjectTypeEnum::SIMULATION || projectTypeEnum == ProjectTypeEnum::TRACE);
}
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_DEFS_H