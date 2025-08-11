/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_RLMICROBATCHCLASSIFIER_H
#define PROFILER_SERVER_RLMICROBATCHCLASSIFIER_H

#include "RLDomainObject.h"
#include "RLProtocolResponse.h"
#include <vector>
#include <queue>

namespace Dic::Module::RL {
using namespace Protocol;
/**
 * @brief:
 *  状态机算法，处理micro batch的分类和聚合
 *  状态： 0--初始状态   1--正向转播阶段  2--反向转播阶段 3--结束
 *  状态转换：
 *        0-->1 : 接收到一个正向算子
 *        0-->3: 完成遍历
 *        1-->1 : 接收到一个正向算子
 *              : 接收到一个反向算子，且在正向算子的时间范围内， count++
 *        1-->2 : 接收到一个反向算子，且在正向算子的时间范围外
 *        0-->3: 完成遍历
 *        2-->2 : 接收到一个反向算子，且count不为0， count--
 *        2-->1 : 接收一个正向算子，count=0
 *        0-->3: 完成遍历
 *  microBatch聚合逻辑(以transformer为例）:
 *      transformer下正向转播算子名称为transformerBlock, 反向算子名称为transformerLayer。
 *      在正向传播阶段一个transformerBlock的范围内可能有n个transformerLayer
 *      对应在反向传播阶段 一个microBatch对应n个transformerLayer
 */

enum State : int {
    Init = 0,
    FP = 1,
    BP = 2
};

class RLMicroBatchClassifier {
public:
    /**
     * @brief: 入口函数，返回处理后的结果
     */
    std::vector<Protocol::RLPipelineNode> ClassifierMicroBatch(std::vector<RLPipelineNode> &nodes);

    void Clear();

private:
    /**
     * @brief 封装前向传播microBatch的生成
     */
    void PushFPNode(std::vector<Protocol::RLPipelineNode> &res);

    /**
     * @brief 封装反向传播microBatch的生成
     */
    void PushBPNode(std::vector<Protocol::RLPipelineNode> &res);

    /**
     * @brief 设置当前的状态和node
     */
    void SetStateAndNode(const RLPipelineNode &node, State state);

    void InitStateProcess(std::vector<Protocol::RLPipelineNode> &res, const Protocol::RLPipelineNode& node);

    void FPStateProcess(std::vector<Protocol::RLPipelineNode> &res, const RLPipelineNode &node);

    void BPStateProcess(std::vector<Protocol::RLPipelineNode> &res, const RLPipelineNode& node);

private:
    State state = Init;      // 当前的状态
    std::queue<int> countQue;   // 记录每个microBatch反向算子聚合的数量
    int count = 0;      // 记录当前microBatch需要聚合的数量
    RLPipelineNode current; // 当前节点
};
}
#endif // PROFILER_SERVER_RLMICROBATCHCLASSIFIER_H
