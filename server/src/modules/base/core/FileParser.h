/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_MODULE_CORE_FILE_PARSER_H
#define DATA_INSIGHT_CORE_MODULE_CORE_FILE_PARSER_H

#include <string>
#include <functional>

namespace Dic {
namespace Module {
class FileParser {
public:
    FileParser() = default;
    virtual ~FileParser() = default;
    virtual bool Parse(const std::vector<std::string> &filePathArr, const std::string &rankId,
                       const std::string &selectedFolder) = 0;
    virtual void SetParseEndCallBack(std::function<void(const std::string, bool result, int executingRankCount)>
            &callback); // fileId, result
    virtual std::string GetError();
    virtual void Reset();

protected:
    std::string error;
    std::function<void(const std::string, bool result, int executingRankCount)> paserEndCallback;
};
} // end of namespace Module
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_MODULE_CORE_FILE_PARSER_H