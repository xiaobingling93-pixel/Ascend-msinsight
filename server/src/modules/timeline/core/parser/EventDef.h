/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_EVENT_DEF_H
#define PROFILER_SERVER_EVENT_DEF_H

#include <string>
#include <optional>

namespace Dic {
namespace Module {
namespace Timeline {
namespace Trace {
struct Event {
    virtual ~Event() = default;
    std::string type;
};

struct Slice : public Event {
    int64_t trackId = 0;
    std::string tid;
    int64_t ts = 0;
    int64_t dur = 0;
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
    std::string tid;
    std::string name;
    std::string pid;
    MetaDataArgs args;
};

struct Flow : public Event {
    int64_t trackId = 0;
    std::string tid;
    int64_t ts = 0;
    std::string pid;
    std::string flowId;
    std::string name;
    std::optional<std::string> cat;
};

struct Counter : public Event {
    std::string name;
    std::string pid;
    int64_t ts = 0;
    std::optional<std::string> cat;
    std::string args;
};
} // end of namespace Trace
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_EVENT_DEF_H