/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include "ProtocolDefs.h"
#include "GlobalProtocolRequest.h"
#include "CancelBaselineHandler.h"
#include "FilesGetHandler.h"
#include "CheckProjectValidHandler.h"
#include "ClearProjectExplorerHandler.h"
#include "DeleteProjectExplorerInfoHandler.h"
#include "GetModuleConfigHandler.h"
#include "HeartCheckHandler.h"
#include "UpdateProjectExplorerInfoHandler.h"

#include "WsSession.h"
#include "ServerDefs.h"
#include "WsSessionManager.h"
#include "WsSessionImpl.h"

using namespace Dic::Server;
using namespace Dic::Module::Global;
using namespace Dic::Module;
using namespace Dic::Protocol;
class GlobalHandlerTest : public ::testing::Test {
public:
    static void SetUpTestCase()
    {
        Dic::Server::WsChannel *ws;
        std::unique_ptr<Dic::Server::WsSessionImpl> session = std::make_unique<Dic::Server::WsSessionImpl>(ws);
        Dic::Server::WsSessionManager::Instance().AddSession(std::move(session));
    }
    static void TearDownTestCase()
    {
        auto session = Dic::Server::WsSessionManager::Instance().GetSession();
        if (session != nullptr) {
            session->SetStatus(WsSession::Status::CLOSED);
            session->WaitForExit();
            Dic::Server::WsSessionManager::Instance().RemoveSession();
        }
    }
protected:
    inline static std::string currPath = Dic::FileUtil::GetCurrPath();
    inline static int index = currPath.find_last_of("server");
};

TEST_F(GlobalHandlerTest, TestCancelBaselineHandler)
{
    std::unique_ptr<Request> requestPtr = std::make_unique<BaselineCancelRequest>();
    CancelBaselineHandler cancelBaselineHandler;
    ASSERT_TRUE(cancelBaselineHandler.HandleRequest(std::move(requestPtr)));
}

TEST_F(GlobalHandlerTest, TestCheckProjectValidHandler)
{
    auto requestPtr = std::make_unique<ProjectCheckValidRequest>();
    requestPtr->params.projectName = "";
    std::string path =  currPath.substr(0, index + 1) + R"(\src\test\test_data\test_rank_0)";
    requestPtr->params.dataPath = {path};
    CheckProjectValidHandler checkProjectValidHandler;
    ASSERT_TRUE(checkProjectValidHandler.HandleRequest(std::move(requestPtr)));
}

TEST_F(GlobalHandlerTest, TestDeleteProjectExplorerHandler)
{
    auto requestPtr = std::make_unique<ProjectExplorerInfoDeleteRequest>();
    requestPtr->params.projectName = "";
    std::string path =  currPath.substr(0, index + 1) + R"(\src\test\test_data\test_rank_0)";
    requestPtr->params.dataPath = {path};
    DeleteProjectExplorerInfoHandler deleteProjectExplorerInfoHandler;
    ASSERT_TRUE(deleteProjectExplorerInfoHandler.HandleRequest(std::move(requestPtr)));
}

TEST_F(GlobalHandlerTest, TestFilesNotExistGetHandler)
{
    auto requestPtr = std::make_unique<FilesGetRequest>();
    requestPtr->params.path = "";
    FilesGetHandler filesGetHandler;
    ASSERT_TRUE(filesGetHandler.HandleRequest(std::move(requestPtr)));
}

TEST_F(GlobalHandlerTest, TestCheckfilesGetHandler)
{
    auto requestPtr = std::make_unique<FilesGetRequest>();
    std::string path =  currPath.substr(0, index + 1) + R"(\src\test\test_data\test_rank_0)";
    requestPtr->params.path = path;
    FilesGetHandler filesGetHandler;
    ASSERT_TRUE(filesGetHandler.HandleRequest(std::move(requestPtr)));
}

TEST_F(GlobalHandlerTest, TestHandleRequest)
{
    auto requestPtr = std::make_unique<HeartCheckRequest>();
    HeartCheckHandler heartCheckHandler;
    ASSERT_TRUE(heartCheckHandler.HandleRequest(std::move(requestPtr)));
}

TEST_F(GlobalHandlerTest, TestUpdateProjectExplorerInfoHandler)
{
    auto requestPtr = std::make_unique<ProjectExplorerInfoUpdateRequest>();
    requestPtr->params.newProjectName = "";
    requestPtr->params.oldProjectName = "";
    UpdateProjectExplorerInfoHandler updateProjectExplorerInfoHandler;
    ASSERT_FALSE(updateProjectExplorerInfoHandler.HandleRequest(std::move(requestPtr)));
}