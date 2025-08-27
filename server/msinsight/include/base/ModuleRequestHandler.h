// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
#ifndef MSINSIGHT_REQUEST_HANDLER_H
#define MSINSIGHT_REQUEST_HANDLER_H
#include "ProtocolUtil.h"
#include "WsSender.h"
namespace Dic::Module {
using namespace Dic::Protocol;
class ModuleRequestHandler {
public:
    ModuleRequestHandler() = default;
    virtual ~ModuleRequestHandler() = default;
    bool HandleRequestEntrance(std::unique_ptr<Request> requestPtr);
    virtual const std::string GetError();
    virtual bool HandleRequest(std::unique_ptr<Request> requestPtr) = 0;
    virtual bool IsAsync();
public:
    static void SetBaseResponse(const Request &request, Response &response);
    static void SetResponseResult(Response &response, bool result, const std::string &errorMsg = "",
                                  const int errorCode = UNKNOW_ERROR);
protected:
    std::string command;
    std::string error;
    std::string moduleName = MODULE_UNKNOWN;
    bool async = true;
};
} // end of namespace Module
#endif // MSINSIGHT_REQUEST_HANDLER_H
