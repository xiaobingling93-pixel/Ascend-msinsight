/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_MODULE_GLOBAL_CONFIG_SET_HANDLER_H
#define DATA_INSIGHT_CORE_MODULE_GLOBAL_CONFIG_SET_HANDLER_H

#include "GlobalHandler.h"

namespace Dic {
namespace Module {
class ConfigSetHandler : public GlobalHandler {
public:
    ConfigSetHandler()
    {
        command = REQ_RES_CONFIG_SET;
    }
    ~ConfigSetHandler() override = default;
    void HandleRequest(std::unique_ptr<Request> requestPtr) override;

};
} // end of namespace Module
} // Dic

#endif // DATA_INSIGHT_CORE_MODULE_GLOBAL_CONFIG_SET_HANDLER_H
