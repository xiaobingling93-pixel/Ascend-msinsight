//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//

#ifndef PROFILER_SERVER_REPOSITORYFACTORYINTERFACE_H
#define PROFILER_SERVER_REPOSITORYFACTORYINTERFACE_H
#include "SliceRepoInterface.h"
#include "CounterRepoInterface.h"
#include "FlowRepoInterface.h"
#include "SimulationSliceRepoInterface.h"
#include "DomainObject.h"
namespace Dic::Module::Timeline {
class RepositoryFactoryInterface {
public:
    virtual ~RepositoryFactoryInterface() = default;
    virtual std::shared_ptr<IBaseSliceRepo> GetSliceRespo(PROCESS_TYPE) = 0;
    virtual std::shared_ptr<SimulationSliceRepoInterface> GetSimulationSliceRespo(PROCESS_TYPE) = 0;
    virtual std::shared_ptr<CounterRepoInterface> GetCounterRespo(PROCESS_TYPE) = 0;
    virtual std::shared_ptr<FlowRepoInterface> GetFlowRespo(PROCESS_TYPE) = 0;
};
}


#endif // PROFILER_SERVER_REPOSITORYFACTORYINTERFACE_H
