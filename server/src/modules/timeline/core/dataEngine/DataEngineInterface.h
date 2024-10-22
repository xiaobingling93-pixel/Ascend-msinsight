//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//
#ifndef PROFILER_SERVER_DATAENGINEINTERFACE_H
#define PROFILER_SERVER_DATAENGINEINTERFACE_H
#include "RepositoryFactoryInterface.h"
#include "SliceRepoInterface.h"
#include "CounterRepoInterface.h"
#include "SimulationSliceRepoInterface.h"

namespace Dic::Module::Timeline {
class DataEngineInterface : public SliceRepoInterface, public FlowRepoInterface, public SimulationSliceRepoInterface {
public:
    ~DataEngineInterface() override = default;
    virtual void SetRepositoryFactory(std::shared_ptr<RepositoryFactoryInterface>) = 0;
};
}


#endif // PROFILER_SERVER_DATAENGINEINTERFACE_H
