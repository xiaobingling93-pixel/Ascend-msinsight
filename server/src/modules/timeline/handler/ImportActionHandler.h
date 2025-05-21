/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_IMPORT_ACTION_HANDLER_H
#define PROFILER_SERVER_IMPORT_ACTION_HANDLER_H

#include <set>
#include <regex>
#include "TimelineRequestHandler.h"
#include "ProjectParserFactory.h"
#include "FileParser.h"

namespace Dic::Module::Timeline {
class ImportActionHandler : public TimelineRequestHandler {
public:
    ImportActionHandler()
    {
        command = Protocol::REQ_RES_IMPORT_ACTION;
        async = false;
    };
    ~ImportActionHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;

private:
    static void SendParseFailEvent(const std::string &message);
    static void LogIfFileNotExist(const Global::ProjectExplorerInfo &projectExplorerInfo);
    static bool TransferProject(ImportActionRequest &request);
    static bool ImportFile(ImportActionRequest &request, std::string &warnMsg);
};
} // end of namespace Dic::Module::Global

#endif // PROFILER_SERVER_IMPORT_ACTION_HANDLER_H
