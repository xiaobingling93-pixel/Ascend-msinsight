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
    std::string token = "Rd``OTxPVKtcJPt]";
    WsSessionManager::Instance().AddSession(token, std::move(session));
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
    int size = 104;
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

TEST(TestUtil, testImportActionHandler)
{
    WsChannel ws;
    std::unique_ptr<WsSession> session = std::make_unique<WsSession>(&ws);
    std::string currPath = Dic::FileUtil::GetCurrPath();
    int index = currPath.find_last_of("server");
    currPath = currPath.substr(0, index + 1);
    std::string token = "Rd``OTxPVKtcJPt]";
    WsSessionManager::Instance().AddSession(token, std::move(session));
    std::string err;
    document_t document(kObjectType);
    document.Parse("{\"id\":4,\"moduleName\":\"timeline\",\"type\":\"request\",\"command\":\"import/action\","
                   "\"params\":{\"token\":\"Rd``OTxPVKtcJPt]\"}}");
    json_t pathArr(kArrayType);
    pathArr.PushBack(json_t().SetString((currPath + "/src/test/test_data").c_str(), document.GetAllocator()),
                     document.GetAllocator());
    JsonUtil::AddMember(document["params"], "path", pathArr, document.GetAllocator());
    std::unique_ptr<Protocol::Request> request = Protocol::ProtocolManager::Instance().
            FromJson(JsonUtil::JsonDump(document), err);
    std::unique_ptr<Module::ProtocolMessage> msg = std::unique_ptr<Module::ProtocolMessage>(request.release());
    auto *reqPtr = dynamic_cast<Protocol::Request *>(msg.release());
    if (reqPtr != nullptr) {
        Module::ModuleManager::Instance().OnDispatchModuleRequest(std::unique_ptr<Protocol::Request>(reqPtr));
    }
    int interval = 1000;
    int maxTimes = 10;
    int count = 0;
    while (true && count < maxTimes) {
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        ParserStatus status0 = ParserStatusManager::Instance().GetParserStatus("0");
        ParserStatus status1 = ParserStatusManager::Instance().GetParserStatus("1");
        ParserStatus clusterStatus = ParserStatusManager::Instance().GetClusterParserStatus();
        if (status0 == ParserStatus::FINISH_ALL && status1 == ParserStatus::FINISH_ALL &&
            clusterStatus == ParserStatus::FINISH) {
            Dic::Server::ServerLog::Info("parse end");
            break;
        }
        count++;
    }
}