/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#include "ResponseQueue.h"

namespace Dic {
namespace Server {
using namespace Dic::Protocol;
ResponseQueue::ResponseQueue() {}

ResponseQueue::~ResponseQueue()
{
    Clear();
}

ResponseQueue &ResponseQueue::operator >> (std::unique_ptr<Protocol::Response> responsePtr)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (responsePtr != nullptr) {
        queue.emplace_back(std::move(responsePtr));
    }
    return *this;
}

std::unique_ptr<Protocol::Response> ResponseQueue::Pop()
{
    std::unique_lock<std::mutex> lock(mutex);
    if (queue.empty()) {
        return nullptr;
    }
    std::unique_ptr<Protocol::Response> responsePtr = std::move(queue.front());
    queue.pop_front();
    return responsePtr;
}

void ResponseQueue::Clear()
{
    std::unique_lock<std::mutex> lock(mutex);
    queue.clear();
}
} // end of namespace Protocol
} // end of namespace Dic