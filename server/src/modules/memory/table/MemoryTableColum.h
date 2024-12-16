/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_TEXTTABLECOLUM_H
#define PROFILER_SERVER_TEXTTABLECOLUM_H
#include <string>
namespace Dic {
namespace Module {
namespace Memory {
namespace OperatorColumn {
constexpr std::string_view ID = "id";
constexpr std::string_view NAME = "name";
constexpr std::string_view SIZE = "size";
constexpr std::string_view ALLOCATION_TIME = "allocation_time";
constexpr std::string_view RELEASE_TIME = "release_time";
constexpr std::string_view DURATION = "duration";
constexpr std::string_view ACTIVE_RELEASE_TIME = "active_release_time";
constexpr std::string_view ACTIVE_DURATION = "active_duration";
constexpr std::string_view ALLOCATION_ALLOCATED = "allocation_allocated";
constexpr std::string_view ALLOCATION_RESERVE = "allocation_reserve";
constexpr std::string_view ALLOCATION_ACTIVE = "allocation_active";
constexpr std::string_view RELEASE_ALLOCATED = "release_allocated";
constexpr std::string_view RELEASE_RESERVE = "release_reserve";
constexpr std::string_view RELEASE_ACTIVE = "release_active";
constexpr std::string_view STREAM = "stream";
}

namespace OpMemoryColumn {
constexpr std::string_view ID = "rowid";
constexpr std::string_view NAME = "name";
constexpr std::string_view SIZE = "size";
constexpr std::string_view ALLOCATION_TIME = "allocationTime";
constexpr std::string_view RELEASE_TIME = "releaseTime";
constexpr std::string_view DURATION = "duration";
constexpr std::string_view ACTIVE_RELEASE_TIME = "activeReleaseTime";
constexpr std::string_view ACTIVE_DURATION = "activeDuration";
constexpr std::string_view ALLOCATION_ALLOCATED = "allocationTotalAllocated";
constexpr std::string_view ALLOCATION_RESERVE = "allocationTotalReserved";
constexpr std::string_view ALLOCATION_ACTIVE = "allocationTotalActive";
constexpr std::string_view RELEASE_ALLOCATED = "releaseTotalAllocated";
constexpr std::string_view RELEASE_RESERVE = "releaseTotalReserved";
constexpr std::string_view RELEASE_ACTIVE = "releaseTotalActive";
constexpr std::string_view STREAM = "streamPtr";
constexpr std::string_view DEVICE_ID = "deviceId";
}
}
}
}
#endif // PROFILER_SERVER_TEXTTABLECOLUM_H
