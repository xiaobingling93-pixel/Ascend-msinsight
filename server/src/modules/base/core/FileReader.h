/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_FILEREADER_H
#define PROFILER_SERVER_FILEREADER_H
#include "IFileReader.h"
namespace Dic {
namespace Module {
class FileReader : public IFileReader {
public:
    ~FileReader() override = default;
    int64_t GetFileSize(const std::string &filePath) override;
};
}
}
#endif // PROFILER_SERVER_FILEREADER_H
