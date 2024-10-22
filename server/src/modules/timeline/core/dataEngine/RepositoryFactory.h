// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.

#ifndef PROFILER_SERVER_REPOSITORYFACTORY_H
#define PROFILER_SERVER_REPOSITORYFACTORY_H
#include <unordered_map>
#include <memory>
#include "RepositoryFactoryInterface.h"
#include "SliceRepoInterface.h"
#include "CounterRepoInterface.h"
#include "DomainObject.h"

namespace Dic::Module::Timeline {
class RepositoryFactory : public RepositoryFactoryInterface {
public:
    static std::shared_ptr<RepositoryFactory> Instance()
    {
        static std::shared_ptr<RepositoryFactory> instance = std::make_shared<RepositoryFactory>();
        return instance;
    }
    RepositoryFactory();
    RepositoryFactory(const RepositoryFactory &) = delete;
    RepositoryFactory &operator = (const RepositoryFactory &) = delete;
    RepositoryFactory(RepositoryFactory &&) = delete;
    RepositoryFactory &operator = (RepositoryFactory &&) = delete;
    std::shared_ptr<SliceRepoInterface> GetSliceRespo(PROCESS_TYPE)override;
    std::shared_ptr<CounterRepoInterface> GetCounterRespo(PROCESS_TYPE)override;
    std::shared_ptr<FlowRepoInterface> GetFlowRespo(PROCESS_TYPE) override;
    std::shared_ptr<SimulationSliceRepoInterface> GetSimulationSliceRespo(PROCESS_TYPE) override;
    ~RepositoryFactory() override;

private:
    std::unordered_map<PROCESS_TYPE, std::shared_ptr<SliceRepoInterface>> sliceRespoMap;
    std::unordered_map<PROCESS_TYPE, std::shared_ptr<CounterRepoInterface>> counterRespoMap;
    std::unordered_map<PROCESS_TYPE, std::shared_ptr<FlowRepoInterface>> flowRespoMap;
    std::unordered_map<PROCESS_TYPE, std::shared_ptr<SimulationSliceRepoInterface>> simulationRespoMap;
};
}


#endif // PROFILER_SERVER_REPOSITORYFACTORY_H
