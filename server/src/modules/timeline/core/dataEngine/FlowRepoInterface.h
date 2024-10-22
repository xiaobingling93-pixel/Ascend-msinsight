//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//

#ifndef PROFILER_SERVER_FLOWREPOINTERFACE_H
#define PROFILER_SERVER_FLOWREPOINTERFACE_H
#include <vector>
#include "DominQuery.h"
#include "DomainObject.h"
namespace Dic::Module::Timeline {
class FlowRepoInterface {
public:
    virtual ~FlowRepoInterface() = default;
    /**
     * 根据连线类别查询所有连线
     * @param flowQuery
     * @param flowPointVec
     */
    virtual void QueryFlowPointByCategory(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec) = 0;
    /* *
     * 查询时间范围内的所有连线点
     * @param flowQuery
     * @param flowPointVec
     */
    virtual void QueryFlowPointByTimeRange(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec) = 0;

    /* *
     * 根据连线点id查询所有连线点
     * @param flowQuery
     * @param flowPointVec
     */
    virtual void QueryFlowPointByFlowId(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec) = 0;
};
}
#endif // PROFILER_SERVER_FLOWREPOINTERFACE_H
