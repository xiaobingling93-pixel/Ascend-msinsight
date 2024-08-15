/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_TEXTTABLECOLUM_H
#define PROFILER_SERVER_TEXTTABLECOLUM_H
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
namespace FlowColumn {
constexpr std::string_view ID = "id";
constexpr std::string_view FLOW_ID = "flow_id";
constexpr std::string_view NAME = "name";
constexpr std::string_view CAT = "cat";
constexpr std::string_view TRACK_ID = "track_id";
constexpr std::string_view TIMESTAMP = "timestamp";
constexpr std::string_view TYPE = "type";
}

namespace ThreadColumn {
constexpr std::string_view TRACK_ID = "track_id";
constexpr std::string_view TID = "tid";
constexpr std::string_view PID = "pid";
constexpr std::string_view THREAD_NAME = "thread_name";
constexpr std::string_view THREAD_SORT_INDEX = "thread_sort_index";
}

namespace TaskColumn {
constexpr std::string_view ROW_ID = "rowid";
constexpr std::string_view TIMESTAMP = "startNs";
constexpr std::string_view ENDTIME = "endNs";
constexpr std::string_view DECICED_ID = "deviceId";
constexpr std::string_view CONNECTION_ID = "connectionId";
constexpr std::string_view GLOBAL_TASK_ID = "globalTaskId";
constexpr std::string_view GLOBAL_PID = "globalPid";
constexpr std::string_view TASK_TYPE = "taskType";
constexpr std::string_view CONTEXT_ID = "contextId";
constexpr std::string_view STREAM_ID = "streamId";
constexpr std::string_view TASK_ID = "taskId";
constexpr std::string_view MODEL_ID = "modelId";
}

namespace CommucationTaskInfoColumn {
constexpr std::string_view ROW_ID = "rowid";
constexpr std::string_view NAME = "name";
constexpr std::string_view GLOBAL_TASK_ID = "globalTaskId";
constexpr std::string_view TASK_TYPE = "taskType";
constexpr std::string_view PLANE_ID = "planeId";
constexpr std::string_view GROUPNAME = "groupName";
constexpr std::string_view NOTIFY_ID = "notifyId";
constexpr std::string_view RDMA_TYPE = "rdmaType";
constexpr std::string_view SRC_RANK = "srcRank";
constexpr std::string_view DST_RANK = "dstRank";
constexpr std::string_view TRANSPORT_TYPE = "transportType";
constexpr std::string_view SIZE = "size";
constexpr std::string_view DATA_TYPE = "dataType";
constexpr std::string_view LINK_TYPE = "linkType";
constexpr std::string_view OP_ID = "opId";
}

namespace CommucationTaskOpColumn {
constexpr std::string_view OP_NAME = "opName";
constexpr std::string_view TIMESTAMP = "startNs";
constexpr std::string_view ENDTIME = "endNs";
constexpr std::string_view CONNECTION_ID = "connectionId";
constexpr std::string_view GROUPNAME = "groupName";
constexpr std::string_view OP_ID = "opId";
constexpr std::string_view RELAY = "relay";
constexpr std::string_view RETRY = "retry";
constexpr std::string_view DATA_TYPE = "dataType";
constexpr std::string_view ALG_TYPE = "algType";
constexpr std::string_view COUNT = "count";
constexpr std::string_view OP_TYPE = "opType";
constexpr std::string_view WAIT_TIME = "waitNs";
}

namespace CannApiColumn {
constexpr std::string_view ID = "connectionId";
constexpr std::string_view TIMESTAMP = "startNs";
constexpr std::string_view ENDTIME = "endNs";
constexpr std::string_view TYPE = "type";
constexpr std::string_view GLOBAL_TID = "globalTid";
constexpr std::string_view NAME = "name";
constexpr std::string_view DEPTH = "depth";
}
}
}
}
#endif // PROFILER_SERVER_TEXTTABLECOLUM_H
