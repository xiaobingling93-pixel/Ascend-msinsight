/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include <fstream>
#include "FileUtil.h"
#include "ServerLog.h"
#include "CommunicationProtocolRequest.h"
#include "WsSessionManager.h"
#include "CommunicatorGroupParserHandler.h"

namespace Dic {
namespace Module {
namespace Communication {
using namespace Dic::Server;
using namespace Dic::Protocol;
using jsonArray = nlohmann::json::array_t;
void CommunicatorGroupParserHandler::HandleRequest(std::unique_ptr<Dic::Protocol::Request> requestPtr)
{
    ServerLog::Info("request to Communication CommunicationGroupParserHandler");
    Protocol::CommunicatorGroupRequest &request =
            dynamic_cast<Protocol::CommunicatorGroupRequest &>(*requestPtr.get());
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Error("Failed to check session token  , command = ", command);
        return;
    }
    std::unique_ptr<Protocol::CommunicatorGroupResponse> responsePtr =
            std::make_unique<Protocol::CommunicatorGroupResponse>();
    CommunicatorGroupResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    // add response to response queue in session
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    SetResponseResult(response, true);
    if (!CommunicatorGroupParserHandler::ParseCommunicatorGroup(request.params.filePath, response.body)) {
        SetResponseResult(response, false);
        ServerLog::Error("Communication CommunicationGroupParserHandler Failed");
    }
    session.OnResponse(std::move(responsePtr));
}

bool CommunicatorGroupParserHandler::ParseCommunicatorGroup(const std::string &filePath,
                                                            Protocol::CommunicatorGroupResBody &resBody)
{
    std::string path = FileUtil::SplicePath(filePath, "cluster_analysis_output");
    path = FileUtil::SplicePath(path, "communication_group.json");
    path = FileUtil::PathPreprocess(path);
    std::ifstream file(path);
    if (file.good()) {
        nlohmann::json fileContent = nlohmann::json::object();
        file >> fileContent;
        nlohmann::json::array_t p2p = fileContent.at("p2p").get<jsonArray>();
        nlohmann::json::array_t collective = fileContent.at("collective").get<jsonArray>();
        auto orderByLenDesAndNumAsc = [](jsonArray a, jsonArray b) {
            if (a.size() == b.size() && a.size() > 0) {
                return a.at(0).get<int>() < b.at(0).get<int>();
            }
            return a.size() > b.size();
        };
        std::sort(p2p.begin(), p2p.end(), orderByLenDesAndNumAsc);
        std::sort(collective.begin(), collective.end(), orderByLenDesAndNumAsc);
        resBody.defaultPPSize = p2p.size();
        for (int i = 0; i < p2p.size(); i++) {
            collective.erase(std::find(collective.begin(), collective.end(), p2p.at(i).get<jsonArray>()));
            auto ranks = p2p.at(i).get<std::vector<int>>();
            auto value = "(" + StringUtil::join<int>(ranks, ", ") + (ranks.size() > 1 ? ")" : ",)");
            resBody.ppGroups.push_back(GroupItem("stage" + std::to_string(i), ranks, value));
        }
        for (int i = 0; i < collective.size(); i++) {
            auto ranks = collective.at(i).get<std::vector<int>>();
            auto value = "(" + StringUtil::join<int>(ranks, ", ") + (ranks.size() > 1 ? ")" : ",)");
            resBody.tpOrDpGroups.push_back(GroupItem("tpOrDp" + std::to_string(i), ranks, value));
        }
        return true;
    }
    return false;
}
} // end of namespace Communication
} // end of namespace Module
} // end of namespace Dic