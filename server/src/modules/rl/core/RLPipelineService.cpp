/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "RLPipelineService.h"
#include "DataBaseManager.h"
#include "RenderEngine.h"
#include "NumberSafeUtil.h"
#include "TrackInfoManager.h"
#include "RLMstxConfigManager.h"
#include "ParserStatusManager.h"

namespace Dic::Module::RL {
using namespace Dic::Module::Timeline;
void RLPipelineService::Clear()
{
    std::lock_guard<std::mutex> lock(mtx);
    minTime = UINT64_MAX;
    maxTime = 0;
    stageTypeList.clear();
    taskPipelineMap.clear();
    microBatchPipelineMap.clear();
}

std::vector<Protocol::RLPipelineNode> RLPipelineService::SearchNode(const std::string &rankId)
{
    // 查找mstx数据
    FullDb::DataType dataType = Timeline::DataBaseManager::Instance().GetDataType();
    std::vector<std::string> taskNameList = RLMstxConfigManager::Instance().GetMstxTaskNameList();
    std::vector<FullDb::CompeteSliceDomain> mstxSliceList =
        FullDb::RenderEngine::Instance()->QueryMstxRLDetail(rankId, dataType, taskNameList);
    if (mstxSliceList.empty()) {
        return {};
    }
    std::vector<Protocol::RLPipelineNode> res;
    std::set<std::string> stageTypeListTemp;
    uint64_t minTimeTemp = UINT64_MAX;
    uint64_t maxTimeTemp = 0;
    for (const auto &item: mstxSliceList) {
        uint64_t duration = NumberSafe::Sub(item.endTime, item.timestamp);
        std::string stageType = RLMstxConfigManager::Instance().GetTaskTypeByName(item.name);
        Protocol::RLPipelineNode node{rankId, "Task", item.timestamp, duration, item.name, stageType};
        res.push_back(node);

        stageTypeListTemp.insert(stageType);
        minTimeTemp = std::min(minTimeTemp, item.timestamp);
        maxTimeTemp = std::max(maxTimeTemp, item.endTime);
    }

    std::lock_guard<std::mutex> lock(mtx);
    stageTypeList.insert(stageTypeListTemp.begin(), stageTypeListTemp.end());
    minTime = std::min(minTimeTemp, minTime);
    maxTime = std::max(maxTimeTemp, maxTime);
    return res;
}

void RLPipelineService::FillPipelineMap(const std::string &originHostName, const std::string &rankId,
                                        const std::vector<Protocol::RLPipelineNode> &pipeline,
                                        std::unordered_map<std::string, RLPipelineItem> &targetMap)
{
    std::string key = originHostName + " " + rankId;
    std::lock_guard<std::mutex> lock(mtx);
    auto taskIt = targetMap.find(key);
    if (taskIt == targetMap.end()) {
        RLPipelineItem taskPipelineItem{pipeline, rankId, originHostName};
        targetMap[key] = taskPipelineItem;
    } else {
        targetMap[key].lists.insert(targetMap[key].lists.end(), pipeline.begin(), pipeline.end());
    }
}

void RLPipelineService::QueryPipelineByRankId(const std::string &rankIdWithHost)
{
    std::string fileId = FullDb::DataBaseManager::Instance().GetFileIdByRankId(rankIdWithHost);
    std::vector<std::string> rankIdWithHostAfterSplit = StringUtil::Split(rankIdWithHost, " ");
    // 获取host，如果rankIdWithHost按空格分割后长度大于1，则倒数第二个是host，否则赋值为空
    std::string hostName = rankIdWithHostAfterSplit.size() > 1 ?
                           rankIdWithHostAfterSplit[rankIdWithHostAfterSplit.size() - 2] : "";
    std::string originHostName = StringUtil::GetOriginHostName(hostName);
    // 获取rankId，如果rankIdWithHost按空格分割后长度大于1，则最后一个是rankId，否则就是rankIdWithHost本身
    std::string rankId = rankIdWithHostAfterSplit.size() > 1 ?
                         rankIdWithHostAfterSplit[rankIdWithHostAfterSplit.size() - 1] : rankIdWithHost;

    // 查询任务维度的数据
    std::vector<Protocol::RLPipelineNode> taskPipelineNodeList = SearchNode(rankIdWithHost);
    FillPipelineMap(originHostName, rankId, taskPipelineNodeList, taskPipelineMap);

    // 查询microBatch维度数据
    std::vector<std::string> taskNames;
    std::transform(taskPipelineNodeList.begin(), taskPipelineNodeList.end(), std::back_inserter(taskNames),
                   [](const RLPipelineNode &node) {
                       return node.name;
                   });
    auto rlMstxConfig = RLMstxConfigManager::Instance().GetMstxConfigByTaskName(taskNames);
    std::vector<Protocol::RLPipelineNode> microBatchNodeList = QueryMicroBatchByTask(fileId,
                                                                                     taskPipelineNodeList,
                                                                                     rlMstxConfig);
    FillPipelineMap(originHostName, rankId, microBatchNodeList, microBatchPipelineMap);
}

bool RLPipelineService::GetPipelineInfo(Protocol::RLPipelineResponse &response)
{
    Clear();
    ThreadPool threadPool = ThreadPool(std::thread::hardware_concurrency());
    for (const auto &rankIdWithHost: FullDb::DataBaseManager::Instance().GetAllRankId()) {
        threadPool.AddTask([this](std::string rankIdWithHost) {
            QueryPipelineByRankId(rankIdWithHost);
            },
            rankIdWithHost);
    }
    threadPool.WaitForAllTasks();
    threadPool.ShutDown();

    std::lock_guard<std::mutex> lock(mtx);
    FillAndProcessPipelineData(taskPipelineMap, response.body.taskData);
    FillAndProcessPipelineData(microBatchPipelineMap, response.body.microBatchData);
    response.body.minTime = minTime;
    response.body.maxTime = maxTime;
    response.body.stageTypeList.insert(response.body.stageTypeList.end(), stageTypeList.begin(), stageTypeList.end());
    return true;
}

std::vector<Protocol::RLPipelineNode> RLPipelineService::QueryMicroBatchByTask(const std::string &fileId,
    const std::vector<Protocol::RLPipelineNode> &taskList, const RLMstxConfig &taskConfig)
{
    std::vector<Protocol::RLPipelineNode> microBatchNodeList;
    for (const auto &item: taskList) {
        auto microBatchUnderTask = QueryMicroBatch(fileId,
            taskConfig, item);
        std::transform(microBatchUnderTask.begin(), microBatchUnderTask.end(), std::back_inserter(microBatchNodeList), [](const auto& item) {
            return item;
        });
    }
    return microBatchNodeList;
}

void RLPipelineService::FillAndProcessPipelineData(std::unordered_map<std::string, RLPipelineItem> &pipelineMap,
                                                   std::vector<RLPipelineItem> &pipelineData)
{
    for (const auto &item: pipelineMap) {
        if (item.second.lists.empty()) {
            continue;
        }
        pipelineData.push_back(item.second);
    }
    std::sort(pipelineData.begin(), pipelineData.end(),
        [](const RLPipelineItem &pipelineA, const RLPipelineItem &pipelineB) {
            if (pipelineA.hostName != pipelineB.hostName) {
                return pipelineA.hostName < pipelineB.hostName;
            }
            if (StringUtil::IsAllDigits(pipelineA.rankId) && StringUtil::IsAllDigits(pipelineB.rankId)) {
                return StringUtil::StringToInt(pipelineA.rankId) < StringUtil::StringToInt(pipelineB.rankId);
            } else {
                return pipelineA.rankId < pipelineB.rankId;
            }
        });
}

std::vector<Protocol::RLPipelineNode> RLPipelineService::QueryMicroBatch(const std::string &fileId,
                                                                         const RLMstxConfig &config,
                                                                         const RLPipelineNode &node)
{
    if (config.taskConfigMap.find(node.name) == config.taskConfigMap.end()) {
        ServerLog::Error("[RL] task config not found when query micro batch");
        return {};
    }
    /*
     * 1. 整理所有microBatch
     * 2. 在task的时间区间内过滤microBatch
     * 3. 状态机算法处理microBatch的时间掩盖问题
     */
    const auto &taskConfig = config.taskConfigMap.at(node.name);
    std::vector<std::string> microBatchNames;
    microBatchNames.reserve(taskConfig.microBatchConfigs.size());
    std::transform(taskConfig.microBatchConfigs.begin(), taskConfig.microBatchConfigs.end(),
                   std::back_inserter(microBatchNames), [](const MicroBatchConfig &item) {
                return item.batchName;
            });
    if (microBatchNames.empty()) {
        return {};
    }
    FullDb::DataType type = DataBaseManager::Instance().GetDataType();
    auto microBatchInDbs = RenderEngine::Instance()->QueryMstxRLDetail(fileId, type, microBatchNames, node.startTime,
                                                                       NumberSafe::Add(node.startTime, node.duration));
    if (microBatchInDbs.empty()) {
        return {};
    }
    std::sort(microBatchInDbs.begin(), microBatchInDbs.end(), [](const CompeteSliceDomain& left, const CompeteSliceDomain& right) {
        if (left.timestamp != right.timestamp) {
            return left.timestamp < right.timestamp;
        } else {
            return left.duration > right.duration;
        }
    });
    std::vector<Protocol::RLPipelineNode> res;
    std::for_each(microBatchInDbs.begin(), microBatchInDbs.end(), [&res, &node, &taskConfig](const auto &sliceItem) {
        RLPipelineNode microBatchNode;
        microBatchNode.stageType = node.stageType;
        microBatchNode.name = sliceItem.name;
        microBatchNode.nodeType = taskConfig.microBatchConfigMap.at(microBatchNode.name).type;
        microBatchNode.startTime = sliceItem.timestamp;
        microBatchNode.duration = sliceItem.endTime - sliceItem.timestamp;
        res.emplace_back(std::move(microBatchNode));
    });

    RLMicroBatchClassifier classifier;
    return classifier.ClassifierMicroBatch(res);
}

RLPipelineService &RLPipelineService::Instance()
{
    static RLPipelineService service;
    return service;
}
}