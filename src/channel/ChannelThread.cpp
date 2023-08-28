/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: declaration of Channel Thread for DIC
 */

#include "UvLoopMgr.h"
#include "ChannelThread.h"

using namespace Dic;

ChannelThread::ChannelThread(const std::shared_ptr<Channel> channelPtr) : channelPtr(channelPtr)
{
    if (this->channelPtr != nullptr) {
        this->channelPtr->SetLoopId(id);
    }
    UvLoopMgr::Instance().LoopInit(id);
}

ChannelThread::~ChannelThread()
{
    this->Stop();
}

void ChannelThread::ThreadFunc(ChannelThread &ct)
{
    if ((ct.channelPtr == nullptr) || ct.channelPtr->IsConnected() || !ct.channelPtr->Connect()) {
        ct.resultCv.notify_one();
        return;
    }
    ct.isStart = true;
    ct.resultCv.notify_one();
    UvLoopMgr::Instance().LoopStart(ct.id);
}

bool ChannelThread::Start()
{
    if (this->isStart) {
        return true;
    }
    threadPtr = std::make_unique<std::thread>(ChannelThread::ThreadFunc, std::ref(*this));
    if (threadPtr == nullptr) {
        return false;
    }
    threadPtr->detach();
    std::unique_lock<std::mutex> lock(resultMutex);
    resultCv.wait_for(lock, std::chrono::milliseconds(resultTimeout), [&] { return this->isStart; });
    // wait for notification from resultCv
    return this->isStart;
}

bool ChannelThread::Stop()
{
    if (!this->isStart) {
        return true;
    }
    this->isStart = false;
    if (this->channelPtr != nullptr) {
        this->channelPtr->Close();
    }
    UvLoopMgr::Instance().LoopStop(this->id);
    return true;
}

bool ChannelThread::IsStart() const
{
    return isStart;
}

const int ChannelThread::ThreadId() const
{
    return this->id;
}

const int ChannelThread::ChannelId() const
{
    if (this->channelPtr == nullptr) {
        return -1;
    }
    return this->channelPtr->Id();
}

const std::shared_ptr<Channel> ChannelThread::GetChannel() const
{
    return this->channelPtr;
}