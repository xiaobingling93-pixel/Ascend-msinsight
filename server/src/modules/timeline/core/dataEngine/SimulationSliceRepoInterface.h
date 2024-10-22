//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//

#ifndef PROFILER_SERVER_SIMULATIONSLICEREPOINTERFACE_H
#define PROFILER_SERVER_SIMULATIONSLICEREPOINTERFACE_H
#include <vector>
#include "DominQuery.h"
#include "DomainObject.h"
namespace Dic::Module::Timeline {
class SimulationSliceRepoInterface {
public:
    virtual ~SimulationSliceRepoInterface() = default;
    virtual void QueryAllFlagSlice(const SliceQuery &sliceQuery,
        std::vector<CompeteSliceDomain> &competeSliceDomain) = 0;
};
}
#endif // PROFILER_SERVER_SIMULATIONSLICEREPOINTERFACE_H
