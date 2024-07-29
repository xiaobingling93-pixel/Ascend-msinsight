/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_FUZZFILEUTIL_H
#define PROFILER_SERVER_FUZZFILEUTIL_H

#include <string>

/**
 * 根据提供基础文件文本内容，随机变异出新的文本内容
 *
 * @param baseFilePath 基础文件路径
 * @param mutationContent 指向变异后文本指针的指针
 * @param mutationContentLength 变异后文本内容的长度
 * @return 如果成功获取变异文本内容，返回0；否则返回1
 */
int GenerateFileMutation(const std::string& baseFilePath, char**  mutationContent, int& mutationContentLength);

#endif // PROFILER_SERVER_FUZZFILEUTIL_H
