/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_MODULE_GLOBAL_H
#define DATA_INSIGHT_CORE_MODULE_GLOBAL_H

#include "BaseModule.h"

namespace Dic {
namespace Module {
class GlobalModule : public BaseModule {
public:
    GlobalModule();
    ~GlobalModule() override;

    void RegisterRequestHandlers() override;
    void OnRequest(std::unique_ptr<Protocol::Request> request) override;
};
} // end of namespace Module
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_MODULE_GLOBAL_H