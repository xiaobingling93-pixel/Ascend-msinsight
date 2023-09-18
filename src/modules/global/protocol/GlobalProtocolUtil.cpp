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
#pragma region <<Response to json>>
template <typename RESPONSE> std::optional<json_t> ToResponseJson(const RESPONSE &response)
{
    ServerLog::Warn("ToResponseJson is not implemented. command:", response.command);
    return std::nullopt;
}

template <> std::optional<json_t> ToResponseJson<TokenCreateResponse>(const TokenCreateResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["createTime"] = response.body.createTime;
    if (response.body.parentToken.has_value() && !response.body.parentToken.value().empty()) {
        json["body"]["parentToken"] = response.body.parentToken.value();
    }
    return json;
}

template <> std::optional<json_t> ToResponseJson<TokenDestroyResponse>(const TokenDestroyResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["destroyTime"] = response.body.destroyTime;
    json["body"]["destroyToken"] = response.body.destroyToken;
    return json;
}

template <> std::optional<json_t> ToResponseJson<TokenCheckResponse>(const TokenCheckResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["checkedToken"] = response.body.checkedToken;
    json["body"]["deadTime"] = response.body.deadTime;
    json["body"]["createTime"] = response.body.createTime;
    return json;
}

json_t FolderToJson(const std::unique_ptr<Folder> &folder)
{
    json_t json;
    json["name"] = folder->name;
    for (const auto &file : folder->childrenFiles) {
        json["childrenFiles"].emplace_back(file);
    }
    for (const auto &childrenFolder : folder->childrenFolders) {
        json["childrenFolders"].emplace_back(FolderToJson(childrenFolder));
    }
    return json;
}

template <> std::optional<json_t> ToResponseJson<FilesGetResponse>(const FilesGetResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["path"] = response.body.path;
    for (const auto &file : response.body.childrenFiles) {
        json["body"]["childrenFiles"].emplace_back(file);
    }
    for (const auto &folder : response.body.childrenFolders) {
        json["body"]["childrenFolders"].emplace_back(FolderToJson(folder));
    }
    return json;
}
#pragma endregion

#pragma region <<Event to json>>
#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic