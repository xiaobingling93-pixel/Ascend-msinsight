// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.

#ifndef PROFILER_SERVER_MOCKFILEREADER_H
#define PROFILER_SERVER_MOCKFILEREADER_H
#include <gmock/gmock.h>
#include "IFileReader.h"
class MockFileReader : public Dic::Module::IFileReader {
public:
    ~MockFileReader() override = default;
    MOCK_METHOD(int64_t, GetFileSize, (const std::string &filePath), (override));
    MOCK_METHOD(std::string, ReadJsonArray, (const std::string &filePath, int64_t startPosition, int64_t endPosition),
    (override));
};
#endif // PROFILER_SERVER_MOCKFILEREADER_H
