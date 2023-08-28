/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Module request handler declaration
 */

#ifndef DATA_INSIGHT_CORE_MODULE_REQUEST_HANDLER_H
#define DATA_INSIGHT_CORE_MODULE_REQUEST_HANDLER_H

#include "Protocol.h"

namespace Dic {
namespace Module {
using namespace Dic::Protocol;
class ModuleRequestHandler {
public:
    ModuleRequestHandler() = default;
    virtual ~ModuleRequestHandler() = default;
    virtual const std::string GetError();
    virtual void HandleRequest(std::unique_ptr<Request> requestPtr) = 0;

protected:
    void SetBaseResponse(const Request &request, Response &response) const;
    void SetResponseResult(Response &response, bool result, const std::string &errorMsg = "",
                           const ErrorCode &errorCode = ErrorCode::UNKNOW_ERROR) const;

    std::string command;
    std::string error;
    ModuleType moduleName = ModuleType::UNKNOWN;
};
} // end of namespace Module
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_MODULE_REQUEST_HANDLER_H
