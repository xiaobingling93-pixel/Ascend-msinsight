/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_IMPORT_ACTION_HANDLER_H
#define PROFILER_SERVER_IMPORT_ACTION_HANDLER_H

#include <set>
#include "AscendRequestHandler.h"

namespace Dic {
namespace Scene {
class ImportActionHandler : public AscendRequestHandler {
public:
    ImportActionHandler()
    {
        command = Protocol::REQ_RES_RESET_WINDOW;
    };
    ~ImportActionHandler() override = default;
    void HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
    static void ParseEndCallBack(const std::string token, const std::string fileId, bool result);
    static void SearchMetaData(const std::string &fileId, std::vector<std::unique_ptr<UnitTrack>> &metaData);

private:
    const std::string_view traceFileName = "trace_view.json";

    void SetParseCallBack(const std::string &token);
    std::vector<std::string> FindTraceFile(const std::string &path);
    std::string GetFileId(const std::string &path);
};
} // end of namespace Scene
} // end of namespace Dic

#endif // PROFILER_SERVER_IMPORT_ACTION_HANDLER_H
