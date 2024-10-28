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

    std::map<std::string, std::vector<std::string>> res;
    for (auto &info : infos) {
        for (const auto &item: info.parseFilePathInfos) {
            res[info.projectName].push_back(item.parseFilePath);
        }
    }
    for (const auto &item : res) {
        Protocol::ProjectDirectoryInfo temp;
        temp.projectName = item.first;
        temp.fileName = item.second;
        response.body.projectDirectoryList.push_back(temp);
    }

    SetResponseResult(response, true);
    session.OnResponse(std::move(responsePtr));
    return true;
}
} // end of namespace Module
} // Dic
