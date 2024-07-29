/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "FuzzFileUtil.h"
#include <fstream>
#include <iostream>
#include "FuzzDefs.h"

int GenerateFileMutation(const std::string& baseFilePath, char**  mutationContent, int& mutationContentLength)
{
    // 打开文件
    std::ifstream baseFile(baseFilePath, std::ios::in);
    if (!baseFile) {
        std::cout << "open base file failed: " << baseFilePath << std::endl;
        return 1;
    }

    // 移动文件指针到文件末尾，确定文件大小
    baseFile.seekg(0, std::ios::end);
    long fileSize = baseFile.tellg();
    baseFile.seekg(0, std::ios::beg);

    // 分配char数组
    std::string buffer;
    buffer.resize(fileSize);

    // 读取文件内容到char数组
    baseFile.read(&buffer[0], fileSize);

    // 检查是否成功读取
    if (!baseFile) {
        std::cout << "read base file failed: " << baseFilePath << std::endl;
        baseFile.close();
        return 1;
    }

    *mutationContent = DT_SetGetString(&g_Element[0], fileSize + 1, UINT32_MAX, &buffer[0]);
    mutationContentLength = DT_GET_MutatedValueLen(&g_Element[0]);

    // 释放资源
    baseFile.close();
    return 0;
}