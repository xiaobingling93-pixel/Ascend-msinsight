/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */
#include "FileParser.h"

namespace Dic {
namespace Module {
std::string FileParser::GetError()
{
    return error;
}

void FileParser::SetParseEndCallBack(std::function<void(const std::string, bool result)> &callback)
{
    paserEndCallback = callback;
}

void FileParser::Reset()
{
    error.clear();
    paserEndCallback == nullptr;
}
} // end of namespace Module
} // end of namespace Dic