/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_IFILEREADER_H
#define PROFILER_SERVER_IFILEREADER_H
#include <string>
namespace Dic {
namespace Module {
class IFileReader {
public:
    virtual int64_t GetFileSize(const std::string &filePath) = 0;
    virtual ~IFileReader() = default;
};
}
}
#endif // PROFILER_SERVER_IFILEREADER_H
