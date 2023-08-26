/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol declaration
 */

#ifndef DIC_PROTOCOL_ENTITY_H
#define DIC_PROTOCOL_ENTITY_H

#include <string>
#include <optional>
#include <utility>
#include <vector>
#include <memory>
#include <map>
#include "ProtocolEnum.h"

namespace Dic {
namespace Protocol {
struct ModuleConfig {
    virtual ~ModuleConfig() = default;
    ModuleType moduleType = ModuleType::UNKNOWN;
};
// global
struct GlobalConfig : public ModuleConfig {
    int maxSessionCount = 20;
};
// timeline
struct TimelineConfig : public ModuleConfig {
    int maxSessionCount = 20;
};

struct UnitMetaData {
    std::string cardId;
};

struct UnitTrackMeatData {
    std::string cardId;
    std::string processId;
    std::string processName;
    std::string label;
    int64_t threadId = 0;
    std::string threadName;
    int maxDepth = 0;
};

struct UnitTrack {
    std::string type;
    UnitTrackMeatData metaData;
    std::vector<std::unique_ptr<UnitTrack>> children;
};

struct Unit {
    std::string type;
    UnitMetaData metadata;
    std::vector<std::unique_ptr<UnitTrack>> children;
};
} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_PROTOCOL_ENTITY_H
