/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "WsSessionManager.h"
#include "ProjectExplorerManager.h"
#include "GetProjectExplorerInfoHandler.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;
using namespace Global;

bool GetProjectExplorerInfoHandler::HandleRequest(std::unique_ptr<Request> requestPtr)
{
    auto &request = dynamic_cast<ProjectExplorerInfoGetRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<ProjectExplorerInfoGetResponse> responsePtr = std::make_unique<ProjectExplorerInfoGetResponse>();
    ProjectExplorerInfoGetResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    std::vector<ProjectExplorerInfo> infos = ProjectExplorerManager::Instance()
            .QueryProjectExplorer("", std::vector<std::string>());
    // adapt one project have two import path, merge by name
    std::map<std::string, std::vector<std::string>> res;
    std::map<std::string, ProjectExplorerInfo> infoMap;
    for (const auto &info: infos) {
        if (infoMap.find(info.projectName) != infoMap.end()) {
            infoMap[info.projectName].MergeProjectExploreInfo(info);
            continue;
        }
        infoMap[info.projectName] = info;
    }
    for (const auto &[name, info]: infoMap) {
        if (info.subParseFileInfo.empty()) {
            continue;
        }
        Protocol::ProjectDirectoryInfo temp;
        temp.projectName = name;
        temp.fileName = info.subParseFileInfo;
        temp.projectTree = info.projectFileTree;
        res.erase(info.projectName);
        response.body.projectDirectoryList.push_back(temp);
    }

    SetResponseResult(response, true);
    session.OnResponse(std::move(responsePtr));
    return true;
}
} // end of namespace Module
} // Dic
