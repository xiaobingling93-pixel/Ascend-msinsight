/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include <gtest/gtest.h>
#include <utility>
#include "IEProtocolResquest.h"
#include "IECurveTableDatailHandler.h"
using namespace Dic::Module::IE;
class IECurveTableDatailHandlerTest : public ::testing::Test {
protected:
    class MockRepo : public Dic::Module::IE::CurveRepo {
    public:
        std::vector<Dic::Module::ColumnAtt> QueryTableInfoByName(const std::string& fileId,
                                                                 const std::string& tableName)
        {
            std::vector<Dic::Module::ColumnAtt> res;
            Dic::Module::ColumnAtt col1;
            col1.key = "lll";
            res.emplace_back(col1);
            Dic::Module::ColumnAtt col2;
            col2.key = "jjj";
            res.emplace_back(col2);
            return res;
        }

        std::vector<std::map<std::string, std::string>> QueryDataByColumnPage(
            const Dic::Module::PageQuery& query, const std::vector<Dic::Module::ColumnAtt>& columns)
        {
            std::vector<std::map<std::string, std::string>> res;
            std::map<std::string, std::string> data1;
            data1["lll"] = "mmm1";
            data1["jjj"] = "ggg2";
            res.emplace_back(data1);
            std::map<std::string, std::string> data2;
            data2["lll"] = "mmm3";
            data2["jjj"] = "ggg4";
            res.emplace_back(data2);
            res.emplace_back(data2);
            return res;
        }

        uint64_t QueryCountByTableName(const Dic::Module::PageQuery& query, const std::string& abscissa = "")
        {
            return 1;  // 1
        }
    };

    class MockIECurveTableDatailHandler : public Dic::Module::IE::IECurveTableDatailHandler {
    public:
        void SetRepo(std::shared_ptr<CurveRepo> mockRepo)
        {
            repo = std::move(mockRepo);
        }
    };
};

TEST_F(IECurveTableDatailHandlerTest, TestHandler)
{
    std::shared_ptr<MockRepo> mockRepo = std::make_shared<MockRepo>();
    std::shared_ptr<MockIECurveTableDatailHandler> handler = std::make_shared<MockIECurveTableDatailHandler>();
    std::unique_ptr<Dic::Protocol::IETableRequest> request = std::make_unique<Dic::Protocol::IETableRequest>();
    request->params.type = "kkkkkkkkkkkkkkkkkk";
    handler->SetRepo(mockRepo);
    auto res = handler->HandleRequest(std::move(request));
    EXPECT_EQ(res, true);
}