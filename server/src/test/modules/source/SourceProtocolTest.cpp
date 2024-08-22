/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "ProtocolManager.h"
#include "SourceProtocolTest.h"

using namespace Dic::Protocol;

class SourceProtocolTest : public ::testing::Test {
protected:
    ProtocolManager *manager;
    std::string error;

protected:
    void SetUp() override
    {
        manager = &ProtocolManager::Instance();
    }
};

TEST_F(SourceProtocolTest, ToCodeFileRequest)
{
    manager->FromJson(TO_CODE_FILE_REQ_JSON, error);
}

TEST_F(SourceProtocolTest, ToApiLineRequest)
{
    manager->FromJson(TO_API_LINE_REQ_JSON, error);
}

TEST_F(SourceProtocolTest, ToApiInstrRequest)
{
    manager->FromJson(TO_API_INSTR_REQ_JSON, error);
}

TEST_F(SourceProtocolTest, ToDetailsBaseInfoRequest)
{
    manager->FromJson(TO_BASE_INFO_REQ_JSON, error);
}

TEST_F(SourceProtocolTest, ToDetailsLoadInfoRequest)
{
    manager->FromJson(TO_LOAD_INFO_REQ_JSON, error);
}

TEST_F(SourceProtocolTest, ToDetailsMemoryGraphRequest)
{
    manager->FromJson(TO_MEMORY_GRAPH_REQ_JSON, error);
}

TEST_F(SourceProtocolTest, ToDetailsMemoryTableRequest)
{
    manager->FromJson(TO_MEMORY_TABLE_REQ_JSON, error);
}

TEST_F(SourceProtocolTest, ToDetailsInterCoreLoadGraphRequest)
{
    manager->FromJson(TO_INTER_CORE_LOAD_GRAPH_JSON, error);
}