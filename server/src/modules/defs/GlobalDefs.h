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
#include "unordered_map"
#include <vector>
#include <algorithm>

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

inline std::string CastParserTypeToStr(ParserType type)
{
    switch (type) {
        case ParserType::DB:
            return "DB";
        case ParserType::BIN:
            return "BIN";
        case ParserType::JSON:
            return "JSON";
        case ParserType::IPYNB:
            return "IPYNB";
        case ParserType::OTHER:
            return "OTHER";
    }
}
enum class ProjectTypeEnum {
    DB = 0,
    BIN = 1,
    IPYNB = 2,
    TEXT_CLUSTER = 3,
    SIMULATION = 4,
    TRACE = 5,
    DB_CLUSTER = 6,
    OTHER = 7
};

inline std::vector<ProjectTypeEnum> projectTypeSupportCompare = {
    ProjectTypeEnum::DB,
    ProjectTypeEnum::TEXT_CLUSTER,
    ProjectTypeEnum::SIMULATION,
    ProjectTypeEnum::TRACE,
    ProjectTypeEnum::DB_CLUSTER,
    ProjectTypeEnum::BIN,
};

static inline bool IsBaseLineConfigurableType(ProjectTypeEnum projectTypeEnum)
{
    auto it = std::find(projectTypeSupportCompare.begin(), projectTypeSupportCompare.end(),
                        projectTypeEnum);
    if (it != projectTypeSupportCompare.end()) {
        return true;
    }
    return false;
}

inline std::unordered_map<ProjectTypeEnum, uint8_t> projectTypeGroup = {
    {ProjectTypeEnum::DB, 1},
    {ProjectTypeEnum::TEXT_CLUSTER, 2},
    {ProjectTypeEnum::SIMULATION, 3},
    {ProjectTypeEnum::TRACE, 2},
    {ProjectTypeEnum::DB_CLUSTER, 1},
};

static inline ParserType coverProjectTypeToParserType(ProjectTypeEnum projectTypeEnum)
{
    switch (projectTypeEnum) {
        case ProjectTypeEnum::DB_CLUSTER:
            return ParserType::DB;
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

static inline bool isFileConflict(ProjectTypeEnum oldProjectType, ProjectTypeEnum newProjectType)
{
    if (projectTypeGroup.count(oldProjectType) == 0 || projectTypeGroup.count(newProjectType) == 0) {
        return true;
    }
    if (projectTypeGroup[oldProjectType] == projectTypeGroup[newProjectType]) {
        return false;
    }
    return true;
}
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_DEFS_H