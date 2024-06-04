/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_REPOSITORYINTERFACE_H
#define PROFILER_SERVER_REPOSITORYINTERFACE_H
#include <vector>
#include <set>
#include "DomainObject.h"
#include "DominQuery.h"
namespace Dic::Module::Timeline {
class Repository {
public:
    /**
     * 根据trackId查询简单算子
     * @param sliceQuery
     * @param sliceVec
     */
    static void QuerySimpleSliceWithOutNameByTrackId(const SliceQuery &sliceQuery, std::vector<SliceDomain> &sliceVec);
    /**
     * 根据TrackId查询所有调用栈算子的id
     * @param sliceQuery
     * @param sliceIds
     */
    static void QuerySliceIdsByCat(const SliceQuery &sliceQuery, std::set<uint64_t> &sliceIds);
};
}
#endif // PROFILER_SERVER_REPOSITORYINTERFACE_H
