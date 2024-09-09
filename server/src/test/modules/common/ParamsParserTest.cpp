/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include <gtest/gtest.h>
#include "ParamsParser.h"
#include "WsServer.h"
#include "ServerLog.h"
#include "WsSessionManager.h"

class ParamsParserTest : public ::testing::Test {
};

TEST_F(ParamsParserTest, testParamsParser)
{
    Dic::Server::ParamsParser::Instance();
    Dic::Server::ParamsParser::Instance().Parse({"exe", "--wsPort=9000", "--wsHost=127.0.0.1", "--scan=3000",
                                                 "--logPath=./", "--logSize=10", "--logLevel=INFO", "--sid=sid"});
    int64_t expectPort = 9000;
    int64_t expectScanPort = 3000;
    int64_t expectLogSize = 10;
    EXPECT_EQ(Dic::Server::ParamsParser::Instance().GetOption().wsPort, expectPort);
    EXPECT_EQ(Dic::Server::ParamsParser::Instance().GetOption().scanPort, expectScanPort);
    EXPECT_EQ(Dic::Server::ParamsParser::Instance().GetOption().logSize, expectLogSize);
    EXPECT_EQ(Dic::Server::ParamsParser::Instance().GetOption().host, "127.0.0.1");
    EXPECT_EQ(Dic::Server::ParamsParser::Instance().GetOption().sid, "sid");
    EXPECT_EQ(Dic::Server::ParamsParser::Instance().GetOption().logLevel, "INFO");
    EXPECT_EQ(Dic::Server::ParamsParser::Instance().GetOption().logPath, "./");
}


TEST_F(ParamsParserTest, testParamsParserErr)
{
    Dic::Server::ParamsParser::Instance();
    bool result = Dic::Server::ParamsParser::Instance().Parse({"exe", "--wsPort=9200"});
    EXPECT_EQ(result, false);
    EXPECT_EQ(Dic::Server::ParamsParser::Instance().GetError(), "ERROR: Port error, port range is 9000-9100.");
    result = Dic::Server::ParamsParser::Instance().Parse({"exe", "--errtest=err"});
    EXPECT_EQ(Dic::Server::ParamsParser::Instance().GetError(), "ERROR: --errtest=err has not been supported.");
    EXPECT_EQ(result, false);
    result = Dic::Server::ParamsParser::Instance().Parse({"exe", "--wsHost=err"});
    EXPECT_EQ(Dic::Server::ParamsParser::Instance().GetError(), "ERROR: Host is not valid.");
    EXPECT_EQ(result, false);
    result = Dic::Server::ParamsParser::Instance().Parse({"exe"});
    EXPECT_EQ(Dic::Server::ParamsParser::Instance().GetError(), "ERROR: Startup parameter count is not enough.");
    EXPECT_EQ(result, false);
}

TEST_F(ParamsParserTest, testServerStart)
{
    std::vector<std::string> args = {"exe", "--wsPort=9003", "--wsHost=127.0.0.1", "--scan=3000",
                                     "--logSize=10", "--logLevel=INFO", "--sid=sid"};
    Dic::Server::ParamsParser::Instance().Parse(args);
    const Dic::Server::ParamsOption &option = Dic::Server::ParamsParser::Instance().GetOption();
    Dic::Server::ServerLog::Initialize(option.logPath, option.logSize, option.logLevel,
                                       std::to_string(option.wsPort));
    Dic::Server::WsServer server(option.host, option.wsPort, option.sid);
    server.Start();
    const int checkInterval = 1000;
    if (server.IsStart()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(checkInterval));
    }
    EXPECT_EQ(server.IsStart(), true);
    server.Stop();
    EXPECT_EQ(server.IsStart(), false);
}

TEST_F(ParamsParserTest, testWsSession)
{
    Dic::Server::WsChannel *ws;
    std::unique_ptr<Dic::Server::WsSession> session = std::make_unique<Dic::Server::WsSession>(ws);
    int waitTime = 10;
    session->WaitForExit(waitTime);
    session->GetChannel();
    session->GetCreateTime();
    session->GetStartTime();
    session->GetStopTime();
    uint32_t deadTime = 100;
    session->SetDeadTime(deadTime);
    EXPECT_EQ(session->GetDeadTime(), deadTime);
    session->OnRequestMessage("data");
    Dic::Protocol::Event event1(R"({"key":"val"})");
    session->SetBundleName("BundleName");
    EXPECT_EQ(session->GetBundleName(), "BundleName");
    session->SetDeviceKey("deviceKey");
    EXPECT_EQ(session->GetDeviceKey(), "deviceKey");
    session->SendEvent(event1);
    Dic::Server::WsSessionManager::Instance().AddSession(std::move(session));
    auto getSession = Dic::Server::WsSessionManager::Instance().GetSession(ws);
    getSession->Stop();
    Dic::Server::WsSessionManager::Instance().ClearSessions();
    Dic::Protocol::Event event("1");
    Dic::Server::WsSessionManager::Instance().OnEventByMainSession(event);
    EXPECT_EQ(getSession == nullptr, false);
}
