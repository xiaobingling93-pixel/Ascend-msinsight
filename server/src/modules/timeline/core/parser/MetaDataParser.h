/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_METADATAPARSER_H
#define PROFILER_SERVER_METADATAPARSER_H
#include <vector>
#include "DomainObject.h"
#include "JsonUtil.h"

namespace Dic::Module::Timeline {
class MetaDataParser {
public:
    static std::vector<ParallelGroupInfo> ParserParallelGroupInfoByFilePath(const std::string &filePath);
    static std::vector<ParallelGroupInfo> ParserParallelGroupInfoByText(const std::string &text);
private:
    const static inline std::string GLOBAL_RANKS = "global_ranks";
    const static inline std::string GLOBAL_NAME = "group_name";
    static std::vector<ParallelGroupInfo> ConvertGroupInfoJsonToObject(const document_t &json);
};
}
#endif // PROFILER_SERVER_METADATAPARSER_H
