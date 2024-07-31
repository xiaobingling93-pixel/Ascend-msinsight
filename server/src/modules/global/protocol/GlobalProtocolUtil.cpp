/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */
#include "pch.h"
#include "GlobalProtocol.h"
#include "GlobalProtocolUtil.h"

namespace Dic {
namespace Protocol {
using namespace Dic::Server;
using namespace rapidjson;
#pragma region <<Response to json>>
template <typename RESPONSE> std::optional<document_t> ToResponseJson(const RESPONSE &response)
{
    ServerLog::Warn("The func that transfomer response to json is not implemented. command:", response.command);
    return std::nullopt;
}

template <> std::optional<document_t> ToResponseJson<TokenHeartCheckResponse>(const TokenHeartCheckResponse &response)
{
    document_t json(kObjectType);
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    return std::move(json);
}

json_t FolderToJson(const std::unique_ptr<Folder> &folder, RAPIDJSON_DEFAULT_ALLOCATOR &allocator)
{
    json_t json(kObjectType);
    JsonUtil::AddMember(json, "name", folder->name, allocator);
    JsonUtil::AddMember(json, "path", folder->path, allocator);
    json_t childrenFiles(kArrayType);
    for (const auto &file : folder->childrenFiles) {
        json_t jFile(kObjectType);
        JsonUtil::AddMember(jFile, "path", file->path, allocator);
        JsonUtil::AddMember(jFile, "name", file->name, allocator);
        childrenFiles.PushBack(jFile, allocator);
    }
    JsonUtil::AddMember(json, "childrenFiles", childrenFiles, allocator);
    json_t childrenFolders(kArrayType);
    for (const auto &childrenFolder : folder->childrenFolders) {
        childrenFolders.PushBack(FolderToJson(childrenFolder, allocator), allocator);
    }
    JsonUtil::AddMember(json, "childrenFolders", childrenFolders, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<FilesGetResponse>(const FilesGetResponse &response)
{
    document_t json(kObjectType);
    json_t body(kObjectType);
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    auto &allocator = json.GetAllocator();
    JsonUtil::AddMember(body, "path", response.body.path, allocator);
    json_t childrenFiles(kArrayType);
    for (const auto &file : response.body.childrenFiles) {
        json_t jFile(kObjectType);
        JsonUtil::AddMember(jFile, "path", file->path, allocator);
        JsonUtil::AddMember(jFile, "name", file->name, allocator);
        childrenFiles.PushBack(jFile, allocator);
    }
    JsonUtil::AddMember(body, "childrenFiles", childrenFiles, allocator);
    JsonUtil::AddMember(body, "exist", response.body.exist, allocator);
    json_t childrenFolders(kArrayType);
    for (const auto &folder : response.body.childrenFolders) {
        childrenFolders.PushBack(FolderToJson(folder, allocator), allocator);
    }
    JsonUtil::AddMember(body, "childrenFolders", childrenFolders, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <>
std::optional<document_t> ToResponseJson<ProjectExplorerInfoUpdateResponse>(
    const ProjectExplorerInfoUpdateResponse &response)
{
    document_t json(kObjectType);
    json_t body(kObjectType);
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    auto &allocator = json.GetAllocator();
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <>
std::optional<document_t> ToResponseJson<ProjectExplorerInfoGetResponse>(const ProjectExplorerInfoGetResponse &response)
{
    document_t json(kObjectType);
    json_t body(kObjectType);
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    auto &allocator = json.GetAllocator();
    json_t projectDirectoryJson(kArrayType);
    for (const auto &item: response.body.projectDirectoryList) {
        json_t temp(kObjectType);
        JsonUtil::AddMember(temp, "projectName", item.projectName, allocator);
        json_t fileNameListJson(kArrayType);
        for (const auto &fileNameItem: item.fileName) {
            fileNameListJson.PushBack(json_t().SetString(fileNameItem.c_str(), allocator), allocator);
        }
        JsonUtil::AddMember(temp, "fileName", fileNameListJson, allocator);
        projectDirectoryJson.PushBack(temp, allocator);
    }
    JsonUtil::AddMember(body, "projectDirectoryList", projectDirectoryJson, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <>
std::optional<document_t> ToResponseJson<ProjectExplorerInfoDeleteResponse>(
    const ProjectExplorerInfoDeleteResponse &response)
{
    document_t json(kObjectType);
    json_t body(kObjectType);
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    auto &allocator = json.GetAllocator();
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <>
std::optional<document_t> ToResponseJson<ProjectConflictCheckResponse>(const ProjectConflictCheckResponse &response)
{
    document_t json(kObjectType);
    json_t body(kObjectType);
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    auto &allocator = json.GetAllocator();
    JsonUtil::AddMember(body, "isConflict", response.body.isConflict, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}
#pragma endregion

#pragma region <<Event to json>>
#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic