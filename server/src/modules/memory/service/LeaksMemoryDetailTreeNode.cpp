/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "LeaksMemoryDetailTreeNode.h"

namespace Dic {
namespace Module {
namespace Memory {
void LeaksMemoryDetailTreeNode::InsertSubNode(const std::string &subNodeName, uint64_t subNodeSize,
                                              const std::string &subNodeTag)
{
    subNodes.emplace(subNodeName, subNodeSize, subNodeTag);
}

bool LeaksMemoryDetailTreeNode::IsValidOwnerTag(const std::string &tag)
{
    if (tag.empty()) {
        return false;
    }
    auto it = LEAKS_MEMORY_ALLOC_OWNER_BASE_TAGS.find(tag);
    if (it != LEAKS_MEMORY_ALLOC_OWNER_BASE_TAGS.end()) {
        return true;
    }
    for (const auto &baseTag : LEAKS_MEMORY_ALLOC_OWNER_BASE_TAGS) {
        if (tag.compare(0, baseTag.size(), baseTag) == 0) {
            return true;
        }
        if (baseTag > tag.substr(0, baseTag.size())) {
            break;
        }
    }
    return false;
}

void LeaksMemoryDetailTreeNode::InsertSubNode(LeaksMemoryDetailTreeNode &subNode)
{
    subNodes.insert(subNode);
}

std::string LeaksMemoryDetailTreeNode::GetNodeNameByOwnerTag(const std::string &tag)
{
    if (LEAKS_MEMORY_ALLOC_OWNER_NAME_MAP.find(tag) != LEAKS_MEMORY_ALLOC_OWNER_NAME_MAP.end()) {
        return LEAKS_MEMORY_ALLOC_OWNER_NAME_MAP.at(tag);
    }
    std::string lcpOwnerTag;
    std::string resultName;
    for (auto &baseTag : LEAKS_MEMORY_ALLOC_OWNER_BASE_TAGS) {
        std::string tempLCP = StringUtil::FindLCP(tag, baseTag);
        if (tempLCP.size() > lcpOwnerTag.size()) {
            lcpOwnerTag = tempLCP;
            resultName = tag.substr(tempLCP.size());
        }
    }
    if (resultName.empty()) {
        resultName = LEAKS_MEMORY_ALLOC_OWNER_DEFAULT_NAME;
    }
    return resultName;
}
}  // Memory
}  // Module
}  // Dic