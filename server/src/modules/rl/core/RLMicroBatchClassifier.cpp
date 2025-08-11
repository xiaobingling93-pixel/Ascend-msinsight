/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "RLMicroBatchClassifier.h"
#include "RLMstxConfigManager.h"
#include "ServerLog.h"

namespace Dic::Module::RL {

std::vector<Protocol::RLPipelineNode> RLMicroBatchClassifier::ClassifierMicroBatch(std::vector<RLPipelineNode> &nodes)
{
    Clear();
    std::vector<Protocol::RLPipelineNode> res;
    for (auto &node: nodes) {
        switch (state) {
            case Init: {
                // 0状态接收一个正向算子，进入状态1，且将当前算子填入结果中
                InitStateProcess(res, node);
                break;
            }
            case FP: {
                // 1状态接收一个正向算子，仍为状态1，且清空count
                FPStateProcess(res, node);
                break;
            }
            case BP: {
                BPStateProcess(res, node);
                break;
            }
            default:
                break;
        }
    }
    res.push_back(current);
    return res;
}

void RLMicroBatchClassifier::PushFPNode(std::vector<Protocol::RLPipelineNode> &res)
{
    res.emplace_back(std::move(current));
    countQue.push(count);
    count = 0;
}

void RLMicroBatchClassifier::PushBPNode(std::vector<Protocol::RLPipelineNode> &res)
{
    res.emplace_back(std::move(current));
    if (!countQue.empty()) {
        count = countQue.front();
        countQue.pop();
    } else {
        count = 0;
    }
}

void RLMicroBatchClassifier::SetStateAndNode(const RLPipelineNode &node, State newState)
{
    current = node;
    this->state = newState;
}

void RLMicroBatchClassifier::Clear()
{
    state = Init;
    count = 0;
    current = RLPipelineNode();
    countQue = std::queue<int>();
}

void RLMicroBatchClassifier::InitStateProcess(std::vector<Protocol::RLPipelineNode> &res,
                                              const Protocol::RLPipelineNode &node)
{
    if (node.nodeType == "FP") {
        SetStateAndNode(node, FP);
        count = 0;
    } else {
        // 状态0 不能接收反向算子
        Server::ServerLog::Error("Receive a backward pass operator at init state, ignored");
    }
}

void RLMicroBatchClassifier::FPStateProcess(std::vector<Protocol::RLPipelineNode> &res, const RLPipelineNode &node)
{
    if (node.nodeType == "FP") {
        PushFPNode(res);
        SetStateAndNode(node, FP);
    } else {
        // 接收一个在正向算子时间范围内的反向算子
        if (node.startTime >= current.startTime &&
            node.duration + node.startTime <= current.duration + current.startTime) {
            count++;
        } else {  // 接收一个正向时间范围外的反向算子, 状态为2
            PushFPNode(res);
            count = countQue.front();
            countQue.pop();
            SetStateAndNode(node, BP);
        }
    }
}

void RLMicroBatchClassifier::BPStateProcess(std::vector<Protocol::RLPipelineNode> &res, const RLPipelineNode &node)
{
    // 状态2收到一个前向算子，状态变为1
    if (node.nodeType == "FP") {
        PushBPNode(res);
        SetStateAndNode(node, FP);
    } else {
        if (count <= 1) {
            PushBPNode(res);
            SetStateAndNode(node, BP);
            return;
        }
        current.duration = (node.startTime + node.duration) - current.startTime;
        count--;
    }
}

}