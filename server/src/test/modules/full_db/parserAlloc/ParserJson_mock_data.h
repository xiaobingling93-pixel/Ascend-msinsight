/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#ifndef PROFILER_SERVER_PARSERJSON_MOCK_DATA_H
#define PROFILER_SERVER_PARSERJSON_MOCK_DATA_H
namespace Dic::Module::ParserJsonMock {
int64_t CheckParseFileInfoSizeWhenOneFileIs20GThenReturnFalseMock(const std::string &filePath)
{
    static uint32_t count = 0;
    const static uint32_t exceptionOrder = 20;
    const static long long FILE_SIZE_COUNT = (JSON_MAX_FILE_SIZE + 1);
    if (count == exceptionOrder) {
        count = 0;
        return FILE_SIZE_COUNT;
    } else {
        count++;
        return 0;
    }
}

int64_t CheckParseFileInfoSizeWhenTotalFileSizeExceed20GThenReturnFalseMock(const std::string &filePath)
{
    static uint32_t count = 0;
    const static uint32_t exceptionOrder = 20;
    const static long long FILE_SIZE_COUNT = 1024 * 1024 * 1024;
    if (count == exceptionOrder) {
        count = 0;
        return 1;
    } else {
        count++;
        return FILE_SIZE_COUNT;
    }
}
}
#endif // PROFILER_SERVER_PARSERJSON_MOCK_DATA_H
