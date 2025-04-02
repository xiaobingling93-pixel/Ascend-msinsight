/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "SystemViewOverallHelper.h"

using namespace Dic::Server;
namespace Dic::Module::Timeline {
void SystemViewOverallHelper::CategorizeComputingEvents()
{
    size_t cpuCubeOpsIndex = 0;
    for (auto &kernelEvent: kernelEvents) {
        if (kernelEvent.flowStartTime == 0) {
            kernelEvent.GetKernelCategories();
            continue;
        }
        // 根据flow start time寻找kernel event对应的python api
        // 注意这里寻找的不是flow start相连的api本身，而是所对应的上层api（方便后续按api名称过滤）
        while (cpuCubeOpsIndex < cpuCubeOps.size() && cpuCubeOps[cpuCubeOpsIndex].end < kernelEvent.flowStartTime) {
            cpuCubeOpsIndex++;
        }
        if (cpuCubeOpsIndex < cpuCubeOps.size() && cpuCubeOps[cpuCubeOpsIndex].start <= kernelEvent.flowStartTime) {
            CpuCubeOpInfo curOp = cpuCubeOps[cpuCubeOpsIndex];
            kernelEvent.pythonApi = curOp.pythonApi;
            kernelEvent.isInBwdTrack = (curOp.trackId == bwdTrackId);
        }
        kernelEvent.GetKernelCategories();
    }
    if (kernelEvents.empty()) {
        ServerLog::Warn("No valid kernels found when querying computing data in system view overall. Please ensure "
            "that the profiling data is set to level 1 or higher and aic_metrics is set to PipeUtilization.");
    }
}

std::vector<SameOperatorsDetails> SystemViewOverallHelper::FilterComputingEventsByCategory(
    const std::vector<std::string> &expectList, uint64_t minTimeStamp, const std::string &opName)
{
    std::vector<SameOperatorsDetails> filteredEvents;
    if (expectList.empty()) {
        return filteredEvents;
    }
    for (auto& kernelEvent : kernelEvents) {
        if (kernelEvent.IsCategoryListEqual(expectList)) {
            if (!opName.empty() && !StringUtil::Contains(StringUtil::ToLower(kernelEvent.opName),
                                                         StringUtil::ToLower(opName))) {  // 按name过滤
                continue;
            }
            SameOperatorsDetails details;
            details.duration = static_cast<uint64_t>(kernelEvent.duration * timeScale);
            details.name = kernelEvent.opName;
            if (kernelEvent.startTime < minTimeStamp) {
                ServerLog::Warn("Unexpected condition: kernel event start time is less than min timestamp. "
                                "Kernel start time: ", kernelEvent.startTime, " Min time stamp: ", minTimeStamp);
                continue;
            }
            details.timestamp = kernelEvent.startTime - minTimeStamp;
            // 临时用于支持db场景跳转
            details.opId = kernelEvent.opId;
            details.depth = kernelEvent.depth;
            filteredEvents.push_back(details);
        }
    }
    return filteredEvents;
}

void SystemViewOverallHelper::AggregateComputingOverallMetrics(std::vector<SystemViewOverallRes> &responseBody)
{
    if (responseBody.empty()) {
        return;
    }
    // responseBody一层级数据中，computing部分位于responseBody[0]
    for (const auto &tmpInfo: kernelEvents) {
        ComputeOverallMetrics(responseBody[0].children, tmpInfo, 0);
    }
    std::sort(responseBody[0].children.begin(), responseBody[0].children.end(), SystemViewOverallRes::CompareByName);
    double otherComputingTime = responseBody[0].totalTime;
    for (auto &item: responseBody[0].children) {
        otherComputingTime -= item.totalTime;
    }
    if (otherComputingTime > 0) {
        SystemViewOverallRes tempRes;
        tempRes.totalTime = otherComputingTime;
        tempRes.name = OVERALL_CAT_OTHER;
        tempRes.ValidateValues();
        tempRes.id = std::to_string(Protocol::SystemViewOverallRes::idCounter.fetch_add(1));
        responseBody[0].children.push_back(tempRes);
    }
    SummarizeSystemViewOverall(responseBody[0], 0);
    responseBody[0].ValidateValues();
}

void SystemViewOverallHelper::SummarizeSystemViewOverall(SystemViewOverallRes &currentRes, uint32_t depth)
{
    // 防止无穷递归
    if (depth > maxDepth) {
        return;
    }

    currentRes.level = depth + 1;
    for (auto &item : currentRes.children) {
        if (currentRes.name == OVERALL_CAT_COMPUTING) {
            if (currentRes.nums <= UINT32_MAX - item.nums) {
                currentRes.nums += item.nums;
            } else {
                currentRes.nums = UINT32_MAX;
                ServerLog::Warn("Add operation failed when summarize overall metrics. Integer overflow.");
            }
            currentRes.max = std::max(currentRes.max, item.max);
            if (item.name != OVERALL_CAT_OTHER) {
                currentRes.min = std::min(currentRes.min, item.min);
            }
        }
        SummarizeSystemViewOverall(item, depth + 1);
        item.ValidateValues();
    }
    // 保留两位小数
    int decimalPlaces = 2;
    currentRes.avg = NumberUtil::DoubleReservedNDigits(
        ((currentRes.nums != 0) ? (currentRes.totalTime / currentRes.nums) : 0), decimalPlaces);
    currentRes.totalTime = NumberUtil::DoubleReservedNDigits(currentRes.totalTime, decimalPlaces);
    // 计算百分比
    currentRes.ratio = NumberUtil::DoubleReservedNDigits(
        (e2eTime != 0) ? (currentRes.totalTime / e2eTime) * PERCENTAGE_RATIO_SCALE : 0, decimalPlaces);
}

void SystemViewOverallHelper::ComputeOverallMetrics(std::vector<SystemViewOverallRes> &resList,
                                                    const OverallTmpInfo& tmpInfo, size_t index)
{
    if (index >= tmpInfo.categoryList.size()) {
        return;
    }
    SystemViewOverallRes &currentRes = FindOrCreateChild(resList, tmpInfo.categoryList[index]);
    UpdateSystemViewResStatus(currentRes, tmpInfo);
    ComputeOverallMetrics(currentRes.children, tmpInfo, index + 1);
}

void SystemViewOverallHelper::UpdateSystemViewResStatus(SystemViewOverallRes& currentRes, const OverallTmpInfo& tmpInfo)
{
    currentRes.totalTime += tmpInfo.duration;
    currentRes.nums++;
    currentRes.max = std::max(tmpInfo.duration, currentRes.max);
    currentRes.min = std::min(tmpInfo.duration, currentRes.min);
}

SystemViewOverallRes &SystemViewOverallHelper::FindOrCreateChild(std::vector<SystemViewOverallRes> &list,
                                                                 const std::string &name)
{
    for (auto &child: list) {
        if (child.name == name) {
            return child;
        }
    }
    SystemViewOverallRes tempRes;
    tempRes.name = name;
    tempRes.id = std::to_string(Protocol::SystemViewOverallRes::idCounter.fetch_add(1));
    list.push_back(tempRes);
    return list.back();
}
}