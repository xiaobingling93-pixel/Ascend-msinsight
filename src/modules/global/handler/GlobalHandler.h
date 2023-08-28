/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_MODULE_GLOBAL_HANDLER_H
#define DATA_INSIGHT_CORE_MODULE_GLOBAL_HANDLER_H

#include "ModuleRequestHandler.h"

namespace Dic {
namespace Module {
class GlobalHandler : public ModuleRequestHandler {
public:
    GlobalHandler()
    {
        moduleName = Protocol::ModuleType::GLOBAL;
    }
    ~GlobalHandler() override = default;
    void HandleRequest(std::unique_ptr<Dic::Protocol::Request> requestPtr) override {}
};
} // end of namespace Module
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_MODULE_GLOBAL_HANDLER_H