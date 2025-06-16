/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_EXPORTOPDETAILSHANDLER_H
#define PROFILER_SERVER_EXPORTOPDETAILSHANDLER_H

#include <fstream>
#include "ModuleRequestHandler.h"
#include "OperatorRequestHandler.h"
#include "OperatorProtocolRequest.h"

namespace Dic::Module::Operator {
    class ExportOpDetailsHandler : public OperatorRequestHandler {
    public:
        ExportOpDetailsHandler()
        {
            command = Protocol::REQ_RES_OPERATOR_MORE_INFO;
        }

        ~ExportOpDetailsHandler() override = default;

        bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
    private:
        bool HandleCompareDataRequest(OperatorExportDetailsRequest &request,
                                      OperatorExportDetailsResponse &responsePtr);
        bool HandleStatisticCompareDataRequest(OperatorExportDetailsRequest &request,
                                               OperatorExportDetailsResponse &response);
        bool HandleNotStatisticCompareDataRequest(OperatorExportDetailsRequest &request,
                                                  OperatorExportDetailsResponse &response);
        bool HandleDataRequest(OperatorExportDetailsRequest &request,
                               OperatorExportDetailsResponse &responsePtr);
        bool HandleStatisticDataRequest(OperatorExportDetailsRequest &request,
                                        OperatorExportDetailsResponse &responsePtr);
        bool HandleNotStatisticDataRequest(OperatorExportDetailsRequest &request,
                                           OperatorExportDetailsResponse &responsePtr);
        bool CreateCsvFile(OperatorExportDetailsRequest &request, OperatorExportDetailsResponse &response);
        bool AppendFileContent(const std::string &str); // 写入文件的内容需校验再传入
        void DestroyFile();
        std::ofstream ofs;
        size_t maxFileSize = 1024 * 1024 * 1024; // 1GB
        size_t currentFileSize = 0;
    };
}

#endif // PROFILER_SERVER_EXPORTOPDETAILSHANDLER_H
