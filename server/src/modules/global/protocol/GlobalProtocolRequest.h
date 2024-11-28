/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol Requests declaration
 */

#ifndef DIC_GLOBAL_PROTOCOL_REQUEST_H
#define DIC_GLOBAL_PROTOCOL_REQUEST_H

#include <string>
#include <optional>
#include "FileUtil.h"
#include "ProtocolDefs.h"
#include "ProtocolMessage.h"

namespace Dic {
namespace Protocol {
// token.heartCheck
struct HeartCheckRequest : public Request {
    HeartCheckRequest() : Request(REQ_RES_HEART_CHECK) {}
};

struct FilesGetParams {
    std::string path;
};

struct FilesGetRequest : public Request {
    FilesGetRequest() : Request(REQ_RES_FILES_GET) {}
    FilesGetParams params;
};

struct ProjectExplorerInfoUpdateParams {
    std::string oldProjectName;
    std::string newProjectName;
};

struct ProjectExplorerInfoUpdateRequest : public Request {
    ProjectExplorerInfoUpdateRequest(): Request(REQ_RES_PROJECT_EXPLORER_UPDATE) {}
    ProjectExplorerInfoUpdateParams params;
};

struct ProjectExplorerInfoGetParams {
};

struct ProjectExplorerInfoGetRequest : public Request {
    ProjectExplorerInfoGetRequest(): Request(REQ_RES_PROJECT_EXPLORER_INFO_GET) {}
    ProjectExplorerInfoGetParams params;
};

struct ProjectExplorerInfoDeleteParams {
    std::string projectName;
    std::vector<std::string> dataPath;
};

struct ProjectExplorerInfoDeleteRequest : public Request {
    ProjectExplorerInfoDeleteRequest(): Request(REQ_RES_PROJECT_EXPLORER_INFO_DELETE) {}
    ProjectExplorerInfoDeleteParams params;
};

struct ProjectExplorerInfoClearParams {
    std::vector<std::string> projectNameList;
};

struct ProjectExplorerInfoClearRequest : public Request {
    ProjectExplorerInfoClearRequest(): Request(REQ_RES_PROJECT_EXPLORER_CLEAR) {}
    ProjectExplorerInfoClearParams params;
};

struct ProjectCheckParams {
    std::string projectName;
    std::vector<std::string> dataPath;
    bool ConvertToRealPath(std::string &errorMsg)
    {
        return FileUtil::ConvertToRealPath(errorMsg, dataPath);
    }
};

struct ProjectCheckValidRequest : public Request {
    ProjectCheckValidRequest(): Request(REQ_RES_PROJECT_VALID_CHECK) {}
    ProjectCheckParams params;
};

struct BaselineSettingParams {
    std::string projectName;
    std::string filePath;
};

struct BaselineSettingRequest : public Request {
    BaselineSettingRequest(): Request(REQ_RES_PROJECT_SET_BASELINE) {}
    BaselineSettingParams params;
};

struct BaselineCancelParams {};
struct BaselineCancelRequest : public Request {
    BaselineCancelRequest(): Request(REQ_RES_PROJECT_CANCEL_BASELINE) {}
    BaselineCancelParams params;
};
} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_GLOBAL_PROTOCOL_REQUEST_H