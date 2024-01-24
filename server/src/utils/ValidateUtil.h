/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_VALIDATEUTIL_H
#define DATA_INSIGHT_CORE_VALIDATEUTIL_H

#include "ServerLog.h"
#include "ExecUtil.h"
#include "FileUtil.h"
#include "FileDef.h"

#if defined(_WIN32)
#include <io.h>
#endif

namespace Dic {
    class ValidateUtil {
    public:
        static bool CheckCsvFile(const std::string& fileName)
        {
            std::ifstream file(fileName);
            if (!file.good()) {
                Server::ServerLog::Error("Cannot get file:", fileName);
                return false;
            }
            if (access(fileName.c_str(), R_OK) == -1) {
                Server::ServerLog::Error("Cannot read file", fileName);
                return false;
            }
            long long size = FileUtil::GetFileSize(fileName.c_str());
            if (size > MAX_FILE_SIZE_2G) {
                Server::ServerLog::Error("file is too big, csv file max is 2G, file:", fileName);
                return false;
            }
            return true;
        }
    };
} // end of namespace Dic
#endif // DATA_INSIGHT_CORE_VALIDATEUTIL_H