/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_MEMORY_TABLECOLUM_H
#define PROFILER_SERVER_MEMORY_TABLECOLUM_H
#include <string>
namespace Dic {
namespace Module {
namespace Memory {
namespace OpMemoryColumn {
constexpr std::string_view ID = "rowid";
constexpr std::string_view NAME = "name";
constexpr std::string_view SIZE = "size";
constexpr std::string_view ALLOCATION_TIME = "allocationTime";
constexpr std::string_view RELEASE_TIME = "releaseTime";
constexpr std::string_view ACTIVE_RELEASE_TIME = "activeReleaseTime";
constexpr std::string_view DURATION = "duration";
constexpr std::string_view ACTIVE_DURATION = "activeDuration";
constexpr std::string_view ALLOCATION_ALLOCATED = "allocationTotalAllocated";
constexpr std::string_view ALLOCATION_RESERVE = "allocationTotalReserved";
constexpr std::string_view ALLOCATION_ACTIVE = "allocationTotalActive";
constexpr std::string_view RELEASE_ALLOCATED = "releaseTotalAllocated";
constexpr std::string_view RELEASE_RESERVE = "releaseTotalReserved";
constexpr std::string_view RELEASE_ACTIVE = "releaseTotalActive";
constexpr std::string_view STREAM = "streamPtr";
constexpr std::string_view DEVICE_ID = "deviceId";

constexpr std::string_view FULL_COLUMNS_WITHOUT_ID[] = {NAME, SIZE,
                                                        ALLOCATION_TIME, RELEASE_TIME, ACTIVE_RELEASE_TIME,
                                                        DURATION, ACTIVE_DURATION,
                                                        ALLOCATION_ALLOCATED, ALLOCATION_RESERVE, ALLOCATION_ACTIVE,
                                                        RELEASE_ALLOCATED, RELEASE_RESERVE, RELEASE_ACTIVE,
                                                        STREAM, DEVICE_ID };
}
}
}
}
#endif // PROFILER_SERVER_MEMORY_TABLECOLUM_H
