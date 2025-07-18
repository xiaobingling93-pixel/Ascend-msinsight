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
#include "JsonUtil.h"
#include <vector>
#include <algorithm>
#include <string>

namespace Dic {
using json_t = rapidjson::Value;
using document_t = rapidjson::Document;
using namespace rapidjson;

enum class ParserType {
    DB = 0,
    BIN = 1,
    JSON = 2,
    IPYNB = 3,
    IE = 4,
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
        default:
            return "";
    }
    return "";
}
enum class ProjectTypeEnum {
    DB = 0,
    BIN = 1,
    IPYNB = 2,
    TEXT_CLUSTER = 3,
    SIMULATION = 4,
    TRACE = 5,
    DB_CLUSTER = 6,
    IE = 7,
    OTHER = 8
};

inline std::vector<ProjectTypeEnum> projectTypeSupportCompare = {
    ProjectTypeEnum::DB,
    ProjectTypeEnum::TEXT_CLUSTER,
    ProjectTypeEnum::SIMULATION,
    ProjectTypeEnum::TRACE,
    ProjectTypeEnum::DB_CLUSTER,
    ProjectTypeEnum::BIN,
};

static inline bool IsSupportCompareType(ProjectTypeEnum projectTypeEnum)
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
        case ProjectTypeEnum::IE:
            return ParserType::IE;
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

static inline bool IsComparable(const ProjectTypeEnum &baselineProjectType, const ProjectTypeEnum &curProjectType)
{
    if (baselineProjectType == curProjectType) {
        return true;
    }
    return !isFileConflict(baselineProjectType, curProjectType);
}

struct RankInfo {
    RankInfo() = default;
    RankInfo(std::string cluster, std::string host, std::string rankId, std::string deviceId, std::string rankName)
        :cluster(std::move(cluster)),
        host(std::move(host)),
        rankId(std::move(rankId)),
        deviceId(std::move(deviceId)),
        rankName(std::move(rankName))
    {
    }

    json_t SerializationToJson(RAPIDJSON_DEFAULT_ALLOCATOR &allocator) const
    {
        json_t rankInfo(kObjectType) ;
        JsonUtil::AddMember(rankInfo, "clusterId", cluster, allocator);
        JsonUtil::AddMember(rankInfo, "host", host, allocator);
        JsonUtil::AddMember(rankInfo, "rankId", rankId, allocator);
        JsonUtil::AddMember(rankInfo, "rankName", rankName, allocator);
        JsonUtil::AddMember(rankInfo, "deviceId", deviceId, allocator);
        return rankInfo;
    }
    std::string cluster;
    std::string host;
    std::string rankId;
    std::string deviceId;
    std::string rankName;
};

struct RankEntry {
    RankEntry() = default;
    RankEntry(std::string fileId, std::string rankId, std::string parseFolder)
        :fileId(std::move(fileId)), rankId(std::move(rankId)), parseFolder(std::move(parseFolder))
    {}
    std::string fileId;
    std::string rankId;
    std::string deviceId;
    std::vector<RankInfo> rankInfo;  // rankInfo：cluster + host + rankId + deviceId
    std::string parseFolder;
    std::vector<std::string> parseFileList;
    bool isDevice{false};
};
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_DEFS_H