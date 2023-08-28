/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_SERVER_RESPONSE_QUEUE_H
#define DATA_INSIGHT_CORE_SERVER_RESPONSE_QUEUE_H

#include <deque>
#include <memory>
#include <mutex>
#include "Protocol.h"

namespace Dic {
namespace Server {
class ResponseQueue {
public:
    ResponseQueue();
    ~ResponseQueue();

    ResponseQueue &operator >> (std::unique_ptr<Protocol::Response> responsePtr);
    std::unique_ptr<Protocol::Response> Pop();
    void Clear();

private:
    std::mutex mutex;
    std::deque<std::unique_ptr<Protocol::Response>> queue;
    std::string error;
};
} // end of namespace Server
} // end of namespace Dic
#endif // DATA_INSIGHT_CORE_SERVER_RESPONSE_QUEUE_H
