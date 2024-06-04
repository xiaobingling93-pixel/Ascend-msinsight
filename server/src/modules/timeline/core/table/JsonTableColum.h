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
const std::string ID = "id";
const std::string TIMESTAMP = "timestamp";
const std::string DURATION = "duration";
const std::string NAME = "name";
const std::string TRACKID = "track_id";
const std::string CAT = "cat";
const std::string ARGS = "args";
const std::string CNAME = "cname";
const std::string ENDTIME = "end_time";
const std::string FLAGID = "flag_id";
}
#pragma endregion
}
}
}
#endif // PROFILER_SERVER_JSONTABLEPO_H
