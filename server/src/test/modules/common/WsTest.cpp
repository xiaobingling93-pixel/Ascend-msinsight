/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "../../TestSuit.cpp"
#include "ServerLog.h"
#include "JsonUtil.h"
#include "ProtocolManager.h"
#include "ModuleManager.h"
#include "WsSessionManager.h"
#include "SummaryProtocol.h"
#include "ParserStatusManager.h"
#include "ProtocolDefs.h"
#include "ClusterParseThreadPoolExecutor.h"
#include "ParamsParser.h"
#include "WsServer.h"
#include "ProtocolMessage.h"

using namespace Dic;

TEST_F(TestSuit, TestAllRequestHandler)
{
    WsChannel *ws;
    std::unique_ptr<WsSession> session = std::make_unique<WsSession>(ws);
    WsSessionManager::Instance().AddSession(std::move(session));
    std::string err = "";
    std::string currPath = Dic::FileUtil::GetCurrPath();
    int index = currPath.find_last_of("server");
    currPath = currPath.substr(0, index + 1);
    std::ifstream file(currPath + R"(/src/test/test_data/request.csv)");
    std::string line;
    int count = 0;
    while (getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        std::unique_ptr<Protocol::Request> request = Protocol::ProtocolManager::Instance().FromJson(line, err);
        std::unique_ptr<Module::ProtocolMessage> msg = std::unique_ptr<Module::ProtocolMessage>(request.release());
        auto *reqPtr = dynamic_cast<Protocol::Request *>(msg.release());
        if (reqPtr != nullptr) {
            Module::ModuleManager::Instance().OnDispatchModuleRequest(std::unique_ptr<Protocol::Request>(reqPtr));
        }
        count++;
        EXPECT_EQ(err, "");
    }
    file.close();
    int size = 134;
    EXPECT_EQ(count, size);
}

TEST_F(TestSuit, TestAllRequestSessionErr)
{
    std::string err = "";
    std::string currPath = Dic::FileUtil::GetCurrPath();
    int index = currPath.find_last_of("server");
    currPath = currPath.substr(0, index + 1);
    std::ifstream file(currPath + R"(/src/test/test_data/request.csv)");
    std::string line;
    int count = 0;
    while (getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        std::unique_ptr<Protocol::Request> request = Protocol::ProtocolManager::Instance().FromJson(line, err);
        std::unique_ptr<Module::ProtocolMessage> msg = std::unique_ptr<Module::ProtocolMessage>(request.release());
        auto *reqPtr = dynamic_cast<Protocol::Request *>(msg.release());
        if (reqPtr != nullptr) {
            Module::ModuleManager::Instance().OnDispatchModuleRequest(std::unique_ptr<Protocol::Request>(reqPtr));
        }
        count++;
        EXPECT_EQ(err, "");
    }
    file.close();
}