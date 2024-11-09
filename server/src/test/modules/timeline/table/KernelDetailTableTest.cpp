/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include <string>
#include "KernelDetailTable.h"
#include "TestCaseDatabaseUtil.h"
using namespace Dic::Protocol;
using namespace Dic::TimeLine::TestCaseUtil;
class KernelDetailTableTest : public ::testing::Test {};

TEST_F(KernelDetailTableTest, testMstxEventsTableColumnMaping)
{
    sqlite3 *db = nullptr;
    std::string sql = "CREATE TABLE kernel_detail (id INTEGER PRIMARY KEY AUTOINCREMENT, rank_id TEXT, step_id TEXT, "
        "name TEXT, op_type TEXT, accelerator_core TEXT, start_time INTEGER, duration INTEGER, wait_time "
        "INTEGER, block_dim INTEGER, input_shapes TEXT, input_data_types TEXT, input_formats TEXT, "
        "output_shapes TEXT, output_data_types TEXT, output_formats TEXT);";
    TestCaseDatabaseUtil::CreateDatabse(db, sql);
    std::string sqlInsert =
        "INSERT INTO \"main\".\"kernel_detail\" (\"id\", \"rank_id\", \"step_id\", \"name\", \"op_type\", "
        "\"accelerator_core\", \"start_time\", \"duration\", \"wait_time\", \"block_dim\", \"input_shapes\", "
        "\"input_data_types\", \"input_formats\", \"output_shapes\", \"output_data_types\", \"output_formats\") VALUES "
        "(1, '0', '1', 'aclnnInplaceZero_ZerosLikeAiCore_ZerosLike', 'ZerosLike', 'AI_VECTOR_CORE', "
        "1724670453431688024, 2339.007, 0, 48, '\"1903865856\"', 'FLOAT16', 'FORMAT_ND', '\"1903865856\"', 'FLOAT16', "
        "'FORMAT_ND');";
    TestCaseDatabaseUtil::InsertData(db, sqlInsert);
    std::vector<KernelDetailPO> kernelDetailPOS;
    Dic::Module::Timeline::KernelDetailTable kernelDetailTable;
    const uint64_t expectSize = 1;
    uint64_t expectId = 1;
    const uint64_t expectStartTime = 1724670453431688024;
    const uint64_t expectduration = 2339;
    const uint64_t expectBlockdim = 48;
    const uint64_t index = 0;
    kernelDetailTable.Select(KernelDetailColumn::ID, KernelDetailColumn::RANK_ID)
        .Select(KernelDetailColumn::STEP_ID, KernelDetailColumn::NAME)
        .Select(KernelDetailColumn::OP_TYPE, KernelDetailColumn::ACCELERATOR_CORE)
        .Select(KernelDetailColumn::START_TIME, KernelDetailColumn::DURATION)
        .Select(KernelDetailColumn::WAIT_TIME, KernelDetailColumn::BLOCK_DIM)
        .Select(KernelDetailColumn::INPUT_SHAPES, KernelDetailColumn::INPUT_DATA_TYPES)
        .Select(KernelDetailColumn::INPUT_FORMATS, KernelDetailColumn::OUTPUT_DATA_TYPES)
        .Select(KernelDetailColumn::OUTPUT_SHAPES, KernelDetailColumn::OUTPUT_FORMATS)
        .ExcuteQuery(db, kernelDetailPOS);
    EXPECT_EQ(kernelDetailPOS.size(), expectSize);
    EXPECT_EQ(kernelDetailPOS[index].id, expectId);
    EXPECT_EQ(kernelDetailPOS[index].rankId, "0");
    EXPECT_EQ(kernelDetailPOS[index].stepId, "1");
    EXPECT_EQ(kernelDetailPOS[index].name, "aclnnInplaceZero_ZerosLikeAiCore_ZerosLike");
    EXPECT_EQ(kernelDetailPOS[index].opType, "ZerosLike");
    EXPECT_EQ(kernelDetailPOS[index].acceleratorCore, "AI_VECTOR_CORE");
    EXPECT_EQ(kernelDetailPOS[index].startTime, expectStartTime);
    EXPECT_EQ(kernelDetailPOS[index].duration, expectduration);
    EXPECT_EQ(kernelDetailPOS[index].waitTime, 0);
    EXPECT_EQ(kernelDetailPOS[index].blockDim, expectBlockdim);
    EXPECT_EQ(kernelDetailPOS[index].inputShapes, "\"1903865856\"");
    EXPECT_EQ(kernelDetailPOS[index].inputDataTypes, "FLOAT16");
    EXPECT_EQ(kernelDetailPOS[index].inputFormats, "FORMAT_ND");
    EXPECT_EQ(kernelDetailPOS[index].outputFormats, "FORMAT_ND");
    EXPECT_EQ(kernelDetailPOS[index].outputShapes, "\"1903865856\"");
    EXPECT_EQ(kernelDetailPOS[index].outputDataTypes, "FLOAT16");
}
