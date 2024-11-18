/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_VALIDATEUTIL_H
#define DATA_INSIGHT_CORE_VALIDATEUTIL_H

#include "ServerLog.h"
#include "SafeFile.h"
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
            std::ifstream file = OpenReadFileSafely(fileName);
            if (!file.good()) {
                Server::ServerLog::Error("Check csv file cannot get file");
                return false;
            }
            std::string filePath = FileUtil::PathPreprocess(fileName);
            if (access(filePath.c_str(), R_OK) == -1) {
                Server::ServerLog::Error("Check csv file cannot read file");
                return false;
            }
            long long size = FileUtil::GetFileSize(filePath.c_str());
            if (size > MAX_FILE_SIZE_2G) {
                Server::ServerLog::Warn("The csv file is too large, and the max size is 2G");
            }
            return true;
        }

        static bool CheckCsvFileList(const std::vector<std::string>& fileNameList)
        {
            for (const auto &fileName: fileNameList) {
                if (!CheckCsvFile(fileName)) {
                    return false;
                }
            }
            return true;
        }
    };
} // end of namespace Dic
#endif // DATA_INSIGHT_CORE_VALIDATEUTIL_H