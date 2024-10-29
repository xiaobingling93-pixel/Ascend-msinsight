/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */
#include "pch.h"
#include "FileReader.h"

int64_t Dic::Module::FileReader::GetFileSize(const std::string &filePath)
{
    return FileUtil::GetFileSize(filePath.c_str());
}
