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
        command = Protocol::REQ_RES_RESET_WINDOW;
    };
    ~ImportActionHandler() override = default;
    void HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;

private:
    const std::string_view traceFileName = "trace_view.json";

    static void SetParseCallBack(const std::string &token);
    static void ParseEndCallBack(const std::string token, const std::string fileId, bool result);
    static void SearchMetaData(const std::string &fileId, std::vector<std::unique_ptr<UnitTrack>> &metaData);
    std::vector<std::string> FindTraceFile(const std::string &path);
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_IMPORT_ACTION_HANDLER_H
