/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_IMPORT_ACTION_HANDLER_H
#define PROFILER_SERVER_IMPORT_ACTION_HANDLER_H

#include <set>
#include "TimelineRequestHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
class ImportActionHandler : public TimelineRequestHandler {
public:
    ImportActionHandler()
    {
        command = Protocol::REQ_RES_IMPORT_ACTION;
    };
    ~ImportActionHandler() override = default;
    void HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;

private:
    static void SetParseCallBack(const std::string &token);
    static void ParseEndCallBack(const std::string token, const std::string fileId, bool result);
    static void SearchMetaData(const std::string &fileId, std::vector<std::unique_ptr<UnitTrack>> &metaData);
    std::vector<std::string> FindTraceFile(const std::string &path);
    std::vector<std::string> FindAllTraceFile(const std::vector<std::string> &pathList);
    bool IsJsonValid(const std::string &fileName);
    void FindAscendFolder(const std::string &path, std::vector<std::string> &traceFiles);
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_IMPORT_ACTION_HANDLER_H
