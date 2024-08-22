/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_MOCKSESSION_H
#define PROFILER_SERVER_MOCKSESSION_H

#include "WsSession.h"

namespace Dic {
namespace Server {
class MockSession : public WsSession {
public:
    MockSession() : WsSession(nullptr) {};
};
}
}

#endif // PROFILER_SERVER_MOCKSESSION_H
