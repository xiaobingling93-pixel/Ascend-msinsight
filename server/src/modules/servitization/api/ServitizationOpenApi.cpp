/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include <queue>
#include "FileUtil.h"
#include "thread"
#include "IEProtocolEvent.h"
#include "WsSender.h"
#include "ServitizationOpenApi.h"
namespace Dic::Module::IE {
bool ServitizationOpenApi::Parse(const std::unordered_map<std::string, std::string>& inputs)
{
    bool res = false;
    for (const auto& item : inputs) {
        Server::ServerLog::Info("input is: ", item.second);
        if (FileUtil::CheckFilePathExist(item.second) && FileUtil::GetFileName(item.second) == IEFileName) {
            ParseSingleFile(item.second, item.first);
            res = true;
            continue;
        }
        if (!FileUtil::IsFolder(item.second)) {
            continue;
        }
        std::string targetFile = FileUtil::SplicePath(item.second, IEFileName);
        if (FileUtil::CheckFilePathExist(targetFile)) {
            ParseSingleFile(targetFile, item.first);
            res = true;
            continue;
        }
    }
    return res;
}

std::vector<TaskInfo> ServitizationOpenApi::ComputeTaskInfo(const std::string& path)
{
    if (ValidIEFile(path)) {
        std::vector<TaskInfo> res;
        std::string folder = FileUtil::GetParentPath(path);
        std::string fileId = context->ComputeFileIdByFolder(folder);
        TaskInfo taskInfo = {fileId, path};
        res.emplace_back(taskInfo);
        return res;
    }
    const int maxDepth = 5;
    std::queue<std::pair<std::string, int>> queue;
    queue.emplace(path, 0);
    std::vector<std::string> validFiles;
    while (!queue.empty()) {
        auto [node, depth] = queue.front();
        queue.pop();
        if (depth > maxDepth) {
            break;
        }
        std::vector<std::string> folders;
        std::vector<std::string> files;
        FileUtil::FindFolders(node, folders, files);
        for (const auto& item : files) {
            std::string absPath = FileUtil::SplicePath(node, item);
            if (ValidIEFile(absPath)) {
                validFiles.emplace_back(absPath);
            }
        }
        for (const auto& item : folders) {
            std::string absPath = FileUtil::SplicePath(node, item);
            queue.emplace(absPath, depth + 1);
        }
    }
    std::vector<TaskInfo> res;
    for (const auto& item : validFiles) {
        std::string fileId = context->ComputeFileIdByFolder(FileUtil::GetParentPath(item));
        TaskInfo taskInfo = {fileId, item};
        res.emplace_back(taskInfo);
    }
    return res;
}

void ServitizationOpenApi::Reset()
{
    context->Reset();
}

bool ServitizationOpenApi::ValidIEFile(const std::string& path)
{
    if (FileUtil::IsFolder(path)) {
        return false;
    }
    std::string fileName = FileUtil::GetFileName(path);
    if (fileName == IEFileName) {
        return true;
    }
    return false;
}

void ServitizationOpenApi::ParseSingleFile(const std::string& filePath, const std::string& fileId)
{
    context->InitDataBase(fileId, filePath);
    auto event = std::make_unique<Protocol::ParseStatisticCompletedEvent>();
    event->moduleName = Protocol::MODULE_IE;
    event->result = true;
    std::vector<std::string> fileIds;
    fileIds.push_back(fileId);
    event->rankIds = fileIds;
    event->fileId = filePath;
    SendEvent(std::move(event));
}

bool ServitizationOpenApi::CreateCurve(const std::string& fileId, const std::string& curve)
{
    if (curve.find("_curve' AS ") == std::string::npos) {
        return false;
    }
    context->ExecuteScript(fileId, curve);
    return true;
}
}  // namespace Dic::Module::IE