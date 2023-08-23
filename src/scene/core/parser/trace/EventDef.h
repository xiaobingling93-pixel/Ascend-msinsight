/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_EVENT_DEF_H
#define PROFILER_SERVER_EVENT_DEF_H

#include <string>
#include <optional>

namespace Dic {
namespace Scene {
namespace Core {
namespace Trace {
struct Event {
    virtual ~Event() = default;
    std::string type;
};

struct Slice : public Event {
    int64_t trackId = 0;
    int64_t tid = 0;
    double ts = 0;
    double dur = 0;
    std::string pid;
    std::string name;
    std::optional<std::string> cat;
    std::optional<std::string> args;
};

struct MetaDataArgs {
    std::string name;
    std::string labels;
    int64_t sortIndex = 0;
};

struct MetaData : public Event {
    int64_t trackId = 0;
    int64_t tid = 0;
    std::string name;
    std::string pid;
    MetaDataArgs args;
};

struct Flow : public Event {
    int64_t trackId = 0;
    int64_t tid = 0;
    double ts = 0;
    std::string pid;
    std::string flowId;
    std::string name;
    std::optional<std::string> cat;
};
} // end of namespace Trace
} // end of namespace Core
} // end of namespace Scene
} // end of namespace Dic
#endif // PROFILER_SERVER_EVENT_DEF_H