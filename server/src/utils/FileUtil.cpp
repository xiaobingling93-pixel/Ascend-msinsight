/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "FileUtil.h"

namespace Dic {
    std::string FileUtil::GetCurrPath()
    {
        char currPath[1024];
        std::string strCurrPath;
#ifdef _WIN32
        ::GetModuleFileName(NULL, currPath, MAX_PATH);
        (_tcsrchr(currPath, '\\'))[1] = 0;
        strCurrPath = StringUtil::GbkToUtf8(currPath);
#elif __APPLE__
        uint32_t size = sizeof(currPath);
        if (_NSGetExecutablePath(currPath, &size) == 0) {
            strCurrPath = std::string(dirname(currPath));
        }
#else
        ssize_t len = readlink("/proc/self/exe", currPath, sizeof(currPath) - 1);
        if (len != -1) {
            currPath[len] = '\0';
            strCurrPath = std::string(dirname(currPath));
        }
#endif
        return strCurrPath;
    }
}