/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include <gtest/gtest.h>
#include "GlobalDefs.h"
#include "ProjectParserFactory.h"

using namespace Dic;
using namespace Dic::Module;

class ParserFactoryTest : public ::testing::Test {
};

TEST_F(ParserFactoryTest, GetImportTypeBinTest)
{
    std::pair<std::string, ParserType> result = ParserFactory::GetImportType("/home/user/data/visualize_data.bin");
    std::pair<std::string, ParserType> expect{"/home/user/data/visualize_data.bin", ParserType::BIN};
    EXPECT_EQ(result, expect);
}

TEST_F(ParserFactoryTest, GetImportTypeIpynbTest)
{
    std::string pathList{"/home/user/data/test.ipynb"};
    std::pair<std::string, ParserType> result = ParserFactory::GetImportType(pathList);
    std::pair<std::string, ParserType> expect{pathList, ParserType::IPYNB};
    EXPECT_EQ(result, expect);
}

TEST_F(ParserFactoryTest, GetImportTypeDbTest)
{
#ifdef __linux__
    std::string currPath = Dic::FileUtil::GetCurrPath();
    int index = currPath.find("server");
    const std::string folderPath = currPath.substr(0, index) + "test/data/test/ubuntu_ascend_pt/ASCEND_PROFILER_OUTPUT";
    const std::string dbPath = folderPath + "/ascend_pytorch_profiler_0.db";
    const std::string mkdirCommand = "mkdir -p " + folderPath;
    system(mkdirCommand.c_str());
    const std::string touchCommand = "touch " + dbPath;
    system(touchCommand.c_str());

    std::string pathList1{currPath.substr(0, index) + "test/data/test"};
    std::pair<std::string, ParserType> result1 = ParserFactory::GetImportType(pathList1);
    std::pair<std::string, ParserType> expect1{pathList1, ParserType::DB};
    EXPECT_EQ(result1, expect1);

    std::string pathList2{currPath.substr(0, index) + "test/data/test/ubuntu_ascend_pt"};
    std::pair<std::string, ParserType> result2 = ParserFactory::GetImportType(pathList2);
    std::pair<std::string, ParserType> expect2{pathList2, ParserType::DB};
    EXPECT_EQ(result2, expect2);

    const std::string rmCommand = "rm -rf " + currPath.substr(0, index) + "test/data/test";
    system(rmCommand.c_str());
#endif
}

TEST_F(ParserFactoryTest, GetImportTypeDbClusterTest)
{
#ifdef __linux__
    std::string currPath = Dic::FileUtil::GetCurrPath();
    int index = currPath.find("server");
    const std::string folderPath = currPath.substr(0, index) + "test/data/cluster/cluster_analysis_output";
    const std::string dbPath = folderPath + "/cluster_analysis.db";
    const std::string mkdirCommand = "mkdir -p " + folderPath;
    system(mkdirCommand.c_str());
    const std::string touchCommand = "touch " + dbPath;
    system(touchCommand.c_str());

    std::string pathList1{currPath.substr(0, index) + "test/data/cluster/cluster_analysis_output"};
    std::pair<std::string, ParserType> result1 = ParserFactory::GetImportType(pathList1);
    std::pair<std::string, ParserType> expect1{pathList1, ParserType::DB};
    EXPECT_EQ(result1, expect1);
    std::string pathList2{currPath.substr(0, index) + "test/data/cluster"};
    std::pair<std::string, ParserType> result2 = ParserFactory::GetImportType(pathList2);
    std::pair<std::string, ParserType> expect2{pathList2, ParserType::DB};
    EXPECT_EQ(result2, expect2);

    const std::string rmCommand = "rm -rf " + currPath.substr(0, index) + "test/data/cluster";
    system(rmCommand.c_str());
#endif
}

TEST_F(ParserFactoryTest, GetImportTypeTextTest)
{
#ifdef __linux__
    std::string currPath = Dic::FileUtil::GetCurrPath();
    int index = currPath.find("server");
    const std::string folderPath = currPath.substr(0, index) + "test/data/test/ubuntu_ascend_pt/ASCEND_PROFILER_OUTPUT";
    const std::string dbPath = folderPath + "/trace_view.json";
    const std::string mkdirCommand = "mkdir -p " + folderPath;
    system(mkdirCommand.c_str());
    const std::string touchCommand = "touch " + dbPath;
    system(touchCommand.c_str());

    std::string pathList1{currPath.substr(0, index) + "test/data/test"};
    std::pair<std::string, ParserType> result1 = ParserFactory::GetImportType(pathList1);
    std::pair<std::string, ParserType> expect1{pathList1, ParserType::JSON};
    EXPECT_EQ(result1, expect1);

    std::string pathList2{currPath.substr(0, index) + "test/data/test/ubuntu_ascend_pt"};
    std::pair<std::string, ParserType> result2 = ParserFactory::GetImportType(pathList2);
    std::pair<std::string, ParserType> expect2{pathList2, ParserType::JSON};
    EXPECT_EQ(result2, expect2);

    const std::string rmCommand = "rm -rf " + currPath.substr(0, index) + "test/data/test";
    system(rmCommand.c_str());
#endif
}

TEST_F(ParserFactoryTest, GetImportTypeTextClusterTest)
{
#ifdef __linux__
    std::string currPath = Dic::FileUtil::GetCurrPath();
    int index = currPath.find("server");
    const std::string folderPath = currPath.substr(0, index) + "test/data/cluster/cluster_analysis_output";
    const std::string dbPath = folderPath + "/cluster_communication.json";
    const std::string mkdirCommand = "mkdir -p " + folderPath;
    system(mkdirCommand.c_str());
    const std::string touchCommand = "touch " + dbPath;
    system(touchCommand.c_str());

    std::string pathList1{currPath.substr(0, index) + "test/data/cluster/cluster_analysis_output"};
    std::pair<std::string, ParserType> result1 = ParserFactory::GetImportType(pathList1);
    std::pair<std::string, ParserType> expect1{pathList1, ParserType::JSON};
    EXPECT_EQ(result1, expect1);

    std::string pathList2{currPath.substr(0, index) + "test/data/cluster"};
    std::pair<std::string, ParserType> result2 = ParserFactory::GetImportType(pathList2);
    std::pair<std::string, ParserType> expect2{pathList2, ParserType::JSON};
    EXPECT_EQ(result2, expect2);

    const std::string rmCommand = "rm -rf " + currPath.substr(0, index) + "test/data/cluster";
    system(rmCommand.c_str());
#endif
}

TEST_F(ParserFactoryTest, GetImportTypeOtherTest)
{
#ifdef __linux__
    std::string currPath = Dic::FileUtil::GetCurrPath();
    int index = currPath.find("server");
    const std::string folderPath = currPath.substr(0, index) + "test/data/scalar/scalar_data";
    const std::string dbPath = folderPath + "/tf.event.out.1";
    const std::string mkdirCommand = "mkdir -p " + folderPath;
    system(mkdirCommand.c_str());
    const std::string touchCommand = "touch " + dbPath;
    system(touchCommand.c_str());

    std::string pathList1{currPath.substr(0, index) + "test/data/scalar"};
    std::pair<std::string, ParserType> result1 = ParserFactory::GetImportType(pathList1);
    std::pair<std::string, ParserType> expect1{pathList1, ParserType::OTHER};
    EXPECT_EQ(result1, expect1);

    std::string pathList2{currPath.substr(0, index) + "test/data/scalar_data"};
    std::pair<std::string, ParserType> result2 = ParserFactory::GetImportType(pathList2);
    std::pair<std::string, ParserType> expect2{pathList2, ParserType::OTHER};
    EXPECT_EQ(result2, expect2);

    const std::string rmCommand = "rm -rf " + currPath.substr(0, index) + "test/data/scalar";
    system(rmCommand.c_str());
#endif
}