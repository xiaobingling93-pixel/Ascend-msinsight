/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_LEAKSMEMORYDETAILTREENODE_H
#define PROFILER_SERVER_LEAKSMEMORYDETAILTREENODE_H

#include "pch.h"

namespace Dic {
namespace Module {
namespace Memory {
// Leaks owner可能的表达前缀
// 为便于构造树形结构逻辑Owner, 实际不存在
const std::string LEAKS_MEMORY_ALLOC_OWNER_HAL = "HAL";
const std::string LEAKS_MEMORY_ALLOC_OWNER_HAL_NAME = "Process_Total_Memory_Allocation";
const std::string LEAKS_MEMORY_ALLOC_OWNER_CANN = "CANN";
const std::string LEAKS_MEMORY_ALLOC_OWNER_CANN_NAME = "CANN_Total_Memory_Allocation";
const std::string LEAKS_MEMORY_ALLOC_OWNER_ATB = "ATB";
const std::string LEAKS_MEMORY_ALLOC_OWNER_ATB_NAME = "ATB_Reserved";
const std::string LEAKS_MEMORY_ALLOC_OWNER_MINDSPORE = "MINDSPORE";
const std::string LEAKS_MEMORY_ALLOC_OWNER_MINDSPORE_NAME = "MindSpore_Reserved";
// 只有PTA支持细分
const std::string LEAKS_MEMORY_ALLOC_OWNER_PTA = "PTA";
const std::string LEAKS_MEMORY_ALLOC_OWNER_PTA_NAME = "PTA_Reserved";
// 代表PTA-算子申请
const std::string LEAKS_MEMORY_ALLOC_OWNER_PTA_OPS = "PTA@ops";
const std::string LEAKS_MEMORY_ALLOC_OWNER_PTA_OPS_NAME = "PTA_Operator";
// 代表PTA-模型  还可细分
const std::string LEAKS_MEMORY_ALLOC_OWNER_PTA_MODEL = "PTA@model";
const std::string LEAKS_MEMORY_ALLOC_OWNER_PTA_MODEL_NAME = "PTA_Model";
// 代表PTA-模型-优化器
const std::string LEAKS_MEMORY_ALLOC_OWNER_PTA_MODEL_OPTIMIZER = "PTA@model@optimizer";
const std::string LEAKS_MEMORY_ALLOC_OWNER_PTA_MODEL_OPTIMIZER_NAME = "Optimizer";
// 代表PTA-模型-权重
const std::string LEAKS_MEMORY_ALLOC_OWNER_PTA_MODEL_WEIGHT = "PTA@model@weight";
const std::string LEAKS_MEMORY_ALLOC_OWNER_PTA_MODEL_WEIGHT_NAME = "Weight";
// 代表PTA-模型-梯度
const std::string LEAKS_MEMORY_ALLOC_OWNER_PTA_MODEL_GRADIENT = "PTA@model@gradient";
const std::string LEAKS_MEMORY_ALLOC_OWNER_PTA_MODEL_GRADIENT_NAME = "Gradient";
// 缺省名
const std::string LEAKS_MEMORY_ALLOC_OWNER_DEFAULT_NAME = "Other";
const std::set<std::string> LEAKS_MEMORY_ALLOC_OWNER_BASE_TAGS = {
    LEAKS_MEMORY_ALLOC_OWNER_HAL,
    LEAKS_MEMORY_ALLOC_OWNER_CANN,
    LEAKS_MEMORY_ALLOC_OWNER_ATB,
    LEAKS_MEMORY_ALLOC_OWNER_MINDSPORE,
    LEAKS_MEMORY_ALLOC_OWNER_PTA,
    LEAKS_MEMORY_ALLOC_OWNER_PTA_OPS,
    LEAKS_MEMORY_ALLOC_OWNER_PTA_MODEL,
    LEAKS_MEMORY_ALLOC_OWNER_PTA_MODEL_OPTIMIZER,
    LEAKS_MEMORY_ALLOC_OWNER_PTA_MODEL_WEIGHT,
    LEAKS_MEMORY_ALLOC_OWNER_PTA_MODEL_GRADIENT
};
const std::set<std::string> LEAKS_MEMORY_ALLOC_OWNER_FRAMEWORK_TAGS = {
    LEAKS_MEMORY_ALLOC_OWNER_CANN,
    LEAKS_MEMORY_ALLOC_OWNER_ATB,
    LEAKS_MEMORY_ALLOC_OWNER_MINDSPORE,
    LEAKS_MEMORY_ALLOC_OWNER_PTA
};
const std::unordered_map<std::string, std::string> LEAKS_MEMORY_ALLOC_OWNER_NAME_MAP = {
    {LEAKS_MEMORY_ALLOC_OWNER_HAL, LEAKS_MEMORY_ALLOC_OWNER_HAL_NAME},
    {LEAKS_MEMORY_ALLOC_OWNER_ATB, LEAKS_MEMORY_ALLOC_OWNER_ATB_NAME},
    {LEAKS_MEMORY_ALLOC_OWNER_PTA, LEAKS_MEMORY_ALLOC_OWNER_PTA_NAME},
    {LEAKS_MEMORY_ALLOC_OWNER_MINDSPORE, LEAKS_MEMORY_ALLOC_OWNER_MINDSPORE_NAME},
    {LEAKS_MEMORY_ALLOC_OWNER_CANN, LEAKS_MEMORY_ALLOC_OWNER_CANN_NAME},
    {LEAKS_MEMORY_ALLOC_OWNER_PTA_OPS, LEAKS_MEMORY_ALLOC_OWNER_PTA_OPS_NAME},
    {LEAKS_MEMORY_ALLOC_OWNER_PTA_MODEL, LEAKS_MEMORY_ALLOC_OWNER_PTA_MODEL_NAME},
    {LEAKS_MEMORY_ALLOC_OWNER_PTA_MODEL_OPTIMIZER, LEAKS_MEMORY_ALLOC_OWNER_PTA_MODEL_OPTIMIZER_NAME},
    {LEAKS_MEMORY_ALLOC_OWNER_PTA_MODEL_WEIGHT, LEAKS_MEMORY_ALLOC_OWNER_PTA_MODEL_WEIGHT_NAME},
    {LEAKS_MEMORY_ALLOC_OWNER_PTA_MODEL_GRADIENT, LEAKS_MEMORY_ALLOC_OWNER_PTA_MODEL_GRADIENT_NAME}
};
// 树最大深度
const int MAX_TREE_DEPTH = 8;
class LeaksMemoryDetailTreeNode {
public:
    LeaksMemoryDetailTreeNode() = default;
    LeaksMemoryDetailTreeNode(std::string name, uint64_t size, std::string tag)
        : name(std::move(name)),
          size(size),
          tag(std::move(tag)) {}

    std::string name;
    uint64_t size;
    std::string tag;
    std::set<LeaksMemoryDetailTreeNode> subNodes;

    inline bool operator<(const LeaksMemoryDetailTreeNode& node) const
    {
        return tag < node.tag;
    }

    void InsertSubNode(const std::string &subNodeName, uint64_t subNodeSize, const std::string &subNodeTag);
    void InsertSubNode(LeaksMemoryDetailTreeNode &subNode);
    static bool IsValidOwnerTag(const std::string &tag);
    static std::string GetNodeNameByOwnerTag(const std::string &tag);
};
} // Memory
} // Module
} // Dic
#endif // PROFILER_SERVER_LEAKSMEMORYDETAILTREENODE_H
