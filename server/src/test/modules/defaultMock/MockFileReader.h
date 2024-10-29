// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.

#ifndef PROFILER_SERVER_MOCKFILEREADER_H
#define PROFILER_SERVER_MOCKFILEREADER_H
#include "IFileReader.h"
class MockFileReader : public Dic::Module::IFileReader {
public:
    ~MockFileReader() override = default;
    virtual int64_t GetFileSize(const std::string &filePath) override
    {
        return 0;
    }
};
#endif // PROFILER_SERVER_MOCKFILEREADER_H
