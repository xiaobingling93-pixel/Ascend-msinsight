/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_DBFLOWREPO_H
#define PROFILER_SERVER_DBFLOWREPO_H
#include "HostFlowRepo.h"
#include "DeviceFlowRepo.h"
namespace Dic::Module::Timeline {
class DbFlowRepo : public FlowRepoInterface {
public:
    void QueryFlowPointByCategory(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec) override;
    void QueryFlowPointByTimeRange(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec) override;
    void QueryFlowPointByFlowId(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec) override;
private:
    std::unique_ptr<HostFlowRepo> hostFlowRepo = std::make_unique<HostFlowRepo>();
    std::unique_ptr<DeviceFlowRepo> deviceFlowRepo = std::make_unique<DeviceFlowRepo>();
    void QueryHostToDevice(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec);
    void QueryMsTx(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec);
    void QueryAsyncNpu(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec);
};
}
#endif // PROFILER_SERVER_DBFLOWREPO_H
