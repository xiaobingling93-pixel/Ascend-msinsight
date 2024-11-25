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
    /* *
    * 获取文件大小
    * @param filePath
    * @return
    */
    virtual int64_t GetFileSize(const std::string &filePath) = 0;
    /* *
     * 从文件中读取json数组
     * @param filePath 文件路径
     * @param startPosition 要读取的开始位置
     * @param endPosition 要读取的结束位置
     * @return
     * 如果startPosition和endPosition都为0就是读取整个文件
     * startPosition必须小于等于endPosition
     */
    virtual std::string ReadJsonArray(const std::string &filePath, int64_t startPosition, int64_t endPosition) = 0;
    virtual ~IFileReader() = default;
};
}
}
#endif // PROFILER_SERVER_IFILEREADER_H
