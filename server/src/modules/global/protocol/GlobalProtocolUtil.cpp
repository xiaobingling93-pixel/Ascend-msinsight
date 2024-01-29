/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */
#include "ServerLog.h"
#include "JsonUtil.h"
#include "ProtocolEnumUtil.h"
#include "GlobalProtocol.h"
#include "GlobalProtocolUtil.h"

namespace Dic {
namespace Protocol {
using namespace Dic::Server;
using namespace rapidjson;
#pragma region <<Response to json>>
template <typename RESPONSE> std::optional<document_t> ToResponseJson(const RESPONSE &response)
{
    ServerLog::Warn("ToResponseJson is not implemented. command:", response.command);
    return std::nullopt;
}

template <> std::optional<document_t> ToResponseJson<TokenCreateResponse>(const TokenCreateResponse &response)
{
    document_t json(kObjectType);
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    auto &allocator = json.GetAllocator();
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "createTime", response.body.createTime, allocator);
    JsonUtil::AddMember(body, "token", response.token, allocator);
    if (response.body.parentToken.has_value() && !response.body.parentToken.value().empty()) {
        JsonUtil::AddMember(body, "parentToken", response.body.parentToken.value(), allocator);
    }
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<TokenDestroyResponse>(const TokenDestroyResponse &response)
{
    document_t json(kObjectType);
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    auto &allocator = json.GetAllocator();
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "destroyTime", response.body.destroyTime, allocator);
    JsonUtil::AddMember(body, "destroyToken", response.body.destroyToken, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<TokenCheckResponse>(const TokenCheckResponse &response)
{
    document_t json(kObjectType);
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    auto &allocator = json.GetAllocator();
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "checkedToken", response.body.checkedToken, allocator);
    JsonUtil::AddMember(body, "deadTime", response.body.deadTime, allocator);
    JsonUtil::AddMember(body, "createTime", response.body.createTime, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
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
#pragma endregion

#pragma region <<Event to json>>
#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic