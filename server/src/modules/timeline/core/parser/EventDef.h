/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_EVENT_DEF_H
#define PROFILER_SERVER_EVENT_DEF_H

#include <string>
#include <optional>
#include <map>

namespace Dic {
namespace Module {
namespace Timeline {
namespace Trace {
struct Event {
    virtual ~Event() = default;
    std::string type;
};

struct Slice : public Event {
    uint64_t trackId = 0;
    std::string tid;
    int64_t ts = 0;
    int64_t dur = 0;
    int64_t end = 0;
    std::string pid;
    std::string name;
    std::string processName;
    std::string threadName;
    std::string cname;
    std::optional<std::string> cat;
    std::optional<std::string> args;
    std::string flagId;
};

struct MetaDataArgs {
    std::string name;
    std::string labels;
    int64_t sortIndex = 0;
};

struct MetaData : public Event {
    uint64_t trackId = 0;
    std::string tid;
    std::string name;
    std::string pid;
    MetaDataArgs args;
};

struct ThreadEvent : public Event {
    uint64_t trackId = 0;
    std::string tid;
    std::string pid;
    std::string threadName;
    uint32_t threadSortIndex = 0;
    bool operator < (const ThreadEvent &right) const
    {
        if (trackId < right.trackId) {
            return true;
        }
        return false;
    }

    void SetThreadSortIndex()
    {
        static std::map<std::string, uint32_t> orderMap = { { "SCALAR", 1 }, { "FLOWCTRL", 2 }, { "MTE1", 3 },
                                                            { "CUBE", 4 },   { "FIXP", 5 },     { "MTE2", 6 },
                                                            { "VECTOR", 7 }, { "MTE3", 8 },     { "CACHEMISS", 9 } };
        threadSortIndex = orderMap[threadName];
    }
};

struct ProcessEvent : public Event {
    std::string pid;
    std::string processName;
    bool operator < (const ProcessEvent &right) const
    {
        if (pid < right.pid) {
            return true;
        }
        return false;
    }
};

struct Flow : public Event {
    uint64_t trackId = 0;
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
    std::string tid;
    uint64_t trackId = 0;
};
} // end of namespace Trace
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_EVENT_DEF_H