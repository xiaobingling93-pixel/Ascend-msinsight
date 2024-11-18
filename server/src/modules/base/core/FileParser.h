/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_MODULE_CORE_FILE_PARSER_H
#define DATA_INSIGHT_CORE_MODULE_CORE_FILE_PARSER_H

#include <string>
#include <functional>
#include <map>
#include <atomic>
#include <memory>
#include "ValidateUtil.h"

namespace Dic {
namespace Module {
enum class JsonFormat {
    JSON_ARRAY_FORMAT = 0,
    JSON_OBJECT_FORMAT
};

struct FileProgress {
public:

    FileProgress(uint64_t initialParsedSize, uint64_t initialTotalSize)
        : parsedSize(initialParsedSize), totalSize(initialTotalSize)
    {}

    uint64_t GetParsedSize() const
    {
        return parsedSize.load();
    }

    uint64_t GetTotalSize() const
    {
        return totalSize.load();
    }

    void AddToParsedSize(uint64_t value)
    {
        parsedSize.fetch_add(value);
    }

    int GetProgressPercentage() const
    {
        uint64_t parsed = parsedSize.load();
        uint64_t total = totalSize.load();
        if (total == 0) {
            return 0; // 避免除以0错误
        }

        double percentage = static_cast<double>(parsed) / static_cast<double>(total) * 100.0;
        int res = static_cast<int>(percentage);
        return res > maxPercentage ? maxPercentage : res;
    }

private:
    std::atomic<std::uint64_t> parsedSize;
    std::atomic<std::uint64_t> totalSize;
    const int maxPercentage = 99;
};

class FileParser {
public:
    FileParser() = default;
    virtual ~FileParser() = default;
    virtual bool Parse(const std::vector<std::string> &filePathArr, const std::string &rankId,
        const std::string &selectedFolder) = 0;
    virtual void SetParseEndCallBack(std::function<void(const std::string, bool result, const std::string)> &callback);
    virtual void SetParseProgressCallBack(
        std::function<void(const std::string, uint64_t parsedSize, uint64_t totalSize, int progress)> &callback);

    virtual std::string GetError();
    virtual void Reset();

protected:
    std::string error;
    std::function<void(const std::string, bool result, const std::string)> paserEndCallback;
    std::function<void(const std::string, uint64_t parsedSize, uint64_t totalSize, int progress)> paserProgressCallback;
    std::map<std::string, std::unique_ptr<FileProgress>> fileProgressMap;
};
} // end of namespace Module
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_MODULE_CORE_FILE_PARSER_H