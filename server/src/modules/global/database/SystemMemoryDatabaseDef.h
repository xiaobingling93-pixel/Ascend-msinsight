/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_FILEMENUDATABASEDEF_H
#define PROFILER_SERVER_FILEMENUDATABASEDEF_H

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include "FileUtil.h"
#include "JsonUtil.h"
namespace Dic::Module::Global {
using namespace Dic;

enum ParseFileType:int64_t {
    PROJECT = 0,  // 项目
    CLUSTER = 1, // 集群
    DATA_FILE =  2, // 大于DATA_FILE的都是实际的数据
    RANK = 3, // 卡
    COMPUTE  = 4, // 算子bin文件
    IPYNB    = 5, // jupyterlab文件
};

inline std::string CastParseFileTypeToStr(ParseFileType type)
{
    switch (type) {
        case ParseFileType::PROJECT:
            return "PROJECT";
        case ParseFileType::CLUSTER:
            return "CLUSTER";
        case ParseFileType::DATA_FILE:
            return "DATA_FILE";
        case ParseFileType::RANK:
            return "RANK";
        case ParseFileType::COMPUTE:
            return "COMPUTE";
        case ParseFileType::IPYNB:
            return "IPYNB";
    }
    return "";
}


struct ParseFileInfo {
    int64_t id{-1};
    int64_t projectExplorerId{-1};
    std::string clusterId;
    std::string parseFilePath;
    std::string dbPath;
    std::string rankId;
    std::string deviceId;
    std::string host;
    std::string subId;   // 用于下一级的parseFileInfo去寻找自己的上一级目录
    std::string curDirName;  // 当前目录名
    ParseFileType  type;
    std::vector<std::shared_ptr<ParseFileInfo>> subParseFile;
    inline json_t SerializeToJson(RAPIDJSON_DEFAULT_ALLOCATOR &allocator)
    {
        json_t res(rapidjson::kObjectType);
        JsonUtil::AddMember(res, "clusterId", clusterId, allocator);
        JsonUtil::AddMember(res, "rankId", rankId, allocator);
        JsonUtil::AddMember(res, "host", host, allocator);
        JsonUtil::AddMember(res, "filePath", parseFilePath, allocator);
        JsonUtil::AddMember(res, "type", CastParseFileTypeToStr(type), allocator);
        JsonUtil::AddMember(res, "deviceId", deviceId, allocator);
        JsonUtil::AddMember(res, "fileDir", curDirName, allocator);
        json_t children(rapidjson::kArrayType);
        std::for_each(subParseFile.begin(), subParseFile.end(), [&children, &allocator](const auto &fileInfo) {
            children.PushBack(fileInfo->SerializeToJson(allocator), allocator);
        });
        JsonUtil::AddMember(res, "children", children, allocator);
        return res;
    }
};

struct ProjectExplorerInfo {
    int64_t id = -1;
    std::string projectName;
    std::string fileName;
    std::vector<std::shared_ptr<ParseFileInfo>> subParseFileInfo;   // 项目下待解析的文件路径,不包含非数据文件类型
    std::vector<std::shared_ptr<ParseFileInfo>> projectFileTree;    // 项目下文件树，包含非数据类型和数据文件类型
    std::unordered_map<std::string, std::shared_ptr<ParseFileInfo>> fileInfoMap;   // subId->ParseFileInfo,用于快速检索
    std::string importType;
    std::vector<std::string> dbPath;
    std::string accessTime;
    int64_t projectType;

    inline void AddSubParseFileInfo(const std::string &subId, ParseFileType type,
                                    const std::shared_ptr<ParseFileInfo> &fileInfo)
    {
        auto it = fileInfoMap.find(subId);
        if (it == fileInfoMap.end()) {
            return;
        }
        if (it->second->type != type) {
            return;
        }
        it->second->subParseFile.emplace_back(fileInfo);
        fileInfoMap.emplace(fileInfo->subId, fileInfo);
        if (fileInfo->type > ParseFileType::DATA_FILE) {
            subParseFileInfo.push_back(fileInfo);
        }
    }

    inline void AddSubParseFileInfo(const std::shared_ptr<ParseFileInfo> &fileInfo)
    {
        if (fileInfo->type == ParseFileType::PROJECT) {
            projectFileTree.emplace_back(fileInfo);
            fileInfoMap.emplace(fileInfo->subId, fileInfo);
            return;
        }
        // tree search
        auto curLevel = &projectFileTree;
        while (!curLevel->empty()) {
            if (curLevel->at(0)->type == fileInfo->type) {
                break;
            }
            bool flag = true;
            for (const auto &file: *curLevel) {
                if (StringUtil::StartWith(fileInfo->subId, file->subId)) {  // contains
                    curLevel = &file->subParseFile;
                    flag = false;
                    break;
                }
            }
            if (flag) {
                break;
            }
        }
        curLevel->emplace_back(fileInfo);
        fileInfoMap.emplace(fileInfo->subId, fileInfo);
        if (fileInfo->type > ParseFileType::DATA_FILE) {
            subParseFileInfo.emplace_back(fileInfo);
        }
    }

    inline std::shared_ptr<ParseFileInfo> GetSubParseFileInfo(const std::string &subId, ParseFileType type)
    {
        auto it = fileInfoMap.find(subId);
        if (it == fileInfoMap.end()) {
            return nullptr;
        }
        if (it->second->type != type) {
            return nullptr;
        }
        return it->second;
    }
};


struct BaselineInfo {
    std::string host;
    std::string rankId;
    std::string cardName;
    std::string errorMessage;
    bool isCluster = false;
};
}
#endif // PROFILER_SERVER_FILEMENUDATABASEDEF_H
