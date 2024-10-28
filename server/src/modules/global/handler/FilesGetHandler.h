/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_FILES_GET_HANDLER_H
#define PROFILER_SERVER_FILES_GET_HANDLER_H

#include "GlobalHandler.h"

namespace Dic {
namespace Module {
namespace Global {
class FilesGetHandler : public GlobalHandler {
public:
    FilesGetHandler()
    {
        command = REQ_RES_FILES_GET;
    }
    ~FilesGetHandler() override = default;
    bool HandleRequest(std::unique_ptr<Request> requestPtr) override;
};
} // Global
} // Module
} // Dic

#endif // PROFILER_SERVER_FILES_GET_HANDLER_H
