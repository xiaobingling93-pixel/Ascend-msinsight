/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol Response declaration
 */

#ifndef DIC_GLOBAL_PROTOCOL_RESPONSE_H
#define DIC_GLOBAL_PROTOCOL_RESPONSE_H

#include <vector>
#include "GlobalDefs.h"
#include "ProtocolDefs.h"
#include "ProtocolMessage.h"

namespace Dic {
namespace Protocol {
// token.heartCheck
struct TokenHeartCheckResponse : public Response {
    TokenHeartCheckResponse() : Response(REQ_RES_HEART_CHECK) {};
};

enum class ProjectErrorType {
    NO_ERRORS = 0,
    PROJECT_NAME_CONFLICT = 1,
    IS_UNSAFE_PATH = 2,
    EXISTING_LARGE_FILES = 3,
    TRANSFER_PROJECT = 4,
    EXCEEDS_MXIMUN_LENGTH = 5,
    OTHER = -1
};

struct File {
    File() = default;
    ~File() = default;
    File(const std::string &n, const std::string &p) : name(n), path(p) {}
    std::string name;
    std::string path;
};

struct Folder {
    std::string name;
    std::string path;
    std::vector<std::unique_ptr<Folder>> childrenFolders;
    std::vector<std::unique_ptr<File>> childrenFiles;
};

struct FilesGetResBody {
    std::string path;
    std::vector<std::unique_ptr<Folder>> childrenFolders;
    std::vector<std::unique_ptr<File>> childrenFiles;
    bool exist = false;
};

struct FilesGetResponse : public Response {
    FilesGetResponse() : Response(REQ_RES_FILES_GET) {};
    FilesGetResBody body;
};

struct ModuleConfigGetResponse : public Response {
    ModuleConfigGetResponse() : Response(REQ_RES_GET_MODULE_CONFIG) {};
    std::vector<std::string> configs;
};

struct ProjectExplorerInfoUpdateBody {
};

struct ProjectExplorerInfoUpdateResponse : public Response {
    ProjectExplorerInfoUpdateResponse() : Response(REQ_RES_PROJECT_EXPLORER_UPDATE) {};
    ProjectExplorerInfoUpdateBody body;
};

struct ProjectDirectoryInfo {
    std::string projectName;
    std::vector<std::string> fileName;
};

struct ProjectExplorerInfoGetBody {
    std::vector<ProjectDirectoryInfo> projectDirectoryList;
};

struct ProjectExplorerInfoGetResponse : public Response {
    ProjectExplorerInfoGetResponse() : Response(REQ_RES_PROJECT_EXPLORER_INFO_GET) {};
    ProjectExplorerInfoGetBody body;
};

struct ProjectExplorerInfoDeleteBody {
};

struct ProjectExplorerInfoDeleteResponse : public Response {
    ProjectExplorerInfoDeleteResponse() : Response(REQ_RES_PROJECT_EXPLORER_INFO_DELETE) {};
    ProjectExplorerInfoDeleteBody body;
};

struct ProjectExplorerInfoClearResponse : public Response {
    ProjectExplorerInfoClearResponse() : Response(REQ_RES_PROJECT_EXPLORER_CLEAR) {};
};

struct ProjectCheckBody {
    ProjectErrorType error = ProjectErrorType::NO_ERRORS;
    int result = static_cast<int>(error);
};

struct ProjectCheckValidResponse : public Response {
    ProjectCheckValidResponse() : Response(REQ_RES_PROJECT_VALID_CHECK) {};
    ProjectCheckBody body;
};

struct BaselineSettingBody {
    std::string rankId;
    std::string host;
    std::string cardName;
    std::string errorMessage;
    bool isCluster = false;
};

struct BaselineSettingResponse : public Response {
    BaselineSettingResponse() : Response(REQ_RES_PROJECT_SET_BASELINE) {};
    BaselineSettingBody body;
};

struct BaselineCancelBody {};
struct BaselineCancelResponse : public Response {
    BaselineCancelResponse() : Response(REQ_RES_PROJECT_CANCEL_BASELINE) {};
    BaselineCancelBody body;
};
} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_GLOBAL_PROTOCOL_RESPONSE_H
