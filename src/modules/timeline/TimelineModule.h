//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
//

#ifndef PROFILER_SERVER_TIMELINE_MODULE_H
#define PROFILER_SERVER_TIMELINE_MODULE_H

#include "BaseModule.h"

namespace Dic {
namespace Module {
class TimelineModule : public BaseModule {
public:
    TimelineModule();
    ~TimelineModule() override;

    void RegisterRequestHandlers() override;
    void OnRequest(std::unique_ptr<Protocol::Request> request) override;
};
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_TIMELINE_MODULE_H
