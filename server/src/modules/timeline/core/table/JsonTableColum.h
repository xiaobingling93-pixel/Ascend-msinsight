/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_JSONTABLEPO_H
#define PROFILER_SERVER_JSONTABLEPO_H
#include <string>
namespace Dic {
namespace Module {
namespace Timeline {
#pragma region << Slice Table Info>>
namespace SliceColumn {
constexpr std::string_view ID = "id";
constexpr std::string_view TIMESTAMP = "timestamp";
constexpr std::string_view DURATION = "duration";
constexpr std::string_view NAME = "name";
constexpr std::string_view TRACKID = "track_id";
constexpr std::string_view CAT = "cat";
constexpr std::string_view ARGS = "args";
constexpr std::string_view CNAME = "cname";
constexpr std::string_view ENDTIME = "end_time";
constexpr std::string_view FLAGID = "flag_id";
}
#pragma endregion
}
}
}
#endif // PROFILER_SERVER_JSONTABLEPO_H
