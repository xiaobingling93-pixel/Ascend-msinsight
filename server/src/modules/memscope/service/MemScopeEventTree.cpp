/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "MemScopeEventTree.h"

namespace Dic::Module::MemScope {
void MemScopeMemoryDetailTreeNode::InsertSubNode(const std::string &subNodeName, uint64_t subNodeSize,
                                                 const std::string &subNodeTag)
{
    subNodes.emplace(subNodeName, subNodeSize, subNodeTag);
}

bool MemScopeMemoryDetailTreeNode::IsValidOwnerTag(const std::string &tag)
{
    if (tag.empty()) {
        return false;
    }
    auto it = MEM_SCOPE_ALLOC_OWNER_FIXED_TAGS.find(tag);
    if (it != MEM_SCOPE_ALLOC_OWNER_FIXED_TAGS.end()) {
        return true;
    }
    for (const auto &baseTag : MEM_SCOPE_ALLOC_OWNER_FIXED_TAGS) {
        if (tag.compare(0, baseTag.size(), baseTag) == 0) {
            return true;
        }
        if (baseTag > tag.substr(0, baseTag.size())) {
            break;
        }
    }
    return false;
}

void MemScopeMemoryDetailTreeNode::InsertSubNode(MemScopeMemoryDetailTreeNode &subNode)
{
    subNodes.insert(subNode);
}

std::string MemScopeMemoryDetailTreeNode::GetNodeNameByOwnerTag(const std::string &tag)
{
    if (MEM_SCOPE_ALLOC_OWNER_NAME_MAP.find(tag) != MEM_SCOPE_ALLOC_OWNER_NAME_MAP.end()) {
        return MEM_SCOPE_ALLOC_OWNER_NAME_MAP.at(tag);
    }
    std::string lcpOwnerTag;
    std::string resultName;
    for (auto &baseTag : MEM_SCOPE_ALLOC_OWNER_FIXED_TAGS) {
        std::string tempLCP = StringUtil::FindLCP(tag, baseTag);
        if (tempLCP.size() > lcpOwnerTag.size()) {
            lcpOwnerTag = tempLCP;
            resultName = tag.substr(tempLCP.size());
        }
    }
    if (resultName.empty()) {
        resultName = MEM_SCOPE_ALLOC_OWNER_DEFAULT_NAME;
    }
    return resultName;
}
}  // Dic::Module::MemScope