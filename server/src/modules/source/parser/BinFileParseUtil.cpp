/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "BinFileParseUtil.h"
#include "ServerLog.h"
#include "FileUtil.h"
#include "NumberSafeUtil.h"

namespace Dic::Module::Source {
using namespace Dic::Server;
std::string BinFileParseUtil::GetContentStr(std::ifstream& file, const Position& position, uint64_t maxSize)
{
    if (!file) {
        return "";
    }
    int64_t start = position.startPos;
    int64_t end = position.endPos;
    if (start >= end || start < 0) {
        ServerLog::Error("Invalid start position % and end position % when get content string from bin file",
            start, end);
        return "";
    }
    int64_t dataSize = NumberSafe::Sub(end, start);
    if (IsDataSizeExceedUpperLimit(dataSize, maxSize)) {
        ServerLog::Error("Data size of content exceeds % bytes when get content string from bin file", maxSize);
        return "";
    }

    file.seekg(start, std::ios::beg);

    std::string jsonStr;
    jsonStr.resize(dataSize);
    file.read(&jsonStr[0], dataSize);
    if (!file) {
        ServerLog::Error("Failed to read content str.");
        return "";
    }
    if (!StringUtil::IsUtf8String(jsonStr)) {
        ServerLog::Error("Can't decode a text frame as utf-8, json string is invalid.");
        return "";
    }
    return jsonStr;
}

bool BinFileParseUtil::IsDataSizeExceedUpperLimit(uint64_t realSize, uint64_t upperLimit)
{
    return realSize > upperLimit;
}

std::string BinFileParseUtil::GetSingleContentStrByDataType(std::ifstream &file, DataTypeEnum dataTypeEnum,
    std::map<int, std::vector<Position>> &curBlockMap)
{
    if (!file.is_open()) {
        return "";
    }
    // 从文件获取内容
    std::vector<Position> &baseInfoPos = curBlockMap[static_cast<int>(dataTypeEnum)];
    if (baseInfoPos.empty()) {
        return "";
    }
    return BinFileParseUtil::GetContentStr(file, baseInfoPos[0]);
}
} // Dic::Module::Source