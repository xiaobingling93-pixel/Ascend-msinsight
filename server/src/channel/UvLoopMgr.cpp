/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: implementation of UvLoopMgr
 */
#include "UvLoopMgr.h"

using namespace Dic;

UvLoopMgr::UvLoopMgr() : defaultId(-1)
{
    this->loopMap[defaultId] = uv_default_loop();
}

UvLoopMgr::~UvLoopMgr()
{
    this->AllLoopStop();
}

UvLoopMgr &UvLoopMgr::Instance()
{
    static UvLoopMgr instance;
    return instance;
}

uv_loop_t *UvLoopMgr::Loop(const int &id)
{
    std::lock_guard<std::mutex> lock(loopMutex);
    if (loopMap.count(id) == 0) {
        uv_loop_t *loop = static_cast<uv_loop_t *>(malloc(sizeof *loop));
        if (loop == nullptr) {
            return nullptr;
        }
        if (uv_loop_init(loop) != 0) {
            free(loop);
            return nullptr;
        }
        loopMap[id] = loop;
    }
    return loopMap.at(id);
}

bool UvLoopMgr::LoopInit(const int &id)
{
    uv_loop_t *loop = Loop(id);
    if (loop == nullptr) {
        return false;
    }
    return true;
}

void UvLoopMgr::LoopStart(const int &id)
{
    uv_loop_t *loop = this->Loop(id); // if loop does not exist, new it
    if (loop != nullptr) {
        uv_run(loop, UV_RUN_DEFAULT);
    }
    this->LoopRemove(id);
}

void UvLoopMgr::LoopStop(const int &id)
{
    std::lock_guard<std::mutex> lock(loopMutex);
    if (loopMap.count(id) == 0) {
        return;
    }
    uv_loop_t *loop = loopMap.at(id);
    if (loop == nullptr) {
        return;
    }
    uv_stop(loop);
}

bool UvLoopMgr::AllLoopStop()
{
    std::lock_guard<std::mutex> lock(loopMutex);
    for (auto loopKv : loopMap) {
        uv_loop_t *loop = loopMap.at(loopKv.first);
        if (loop == nullptr) {
            continue;
        }
        if (uv_loop_alive(loop) != 0) {
            uv_stop(loop);
            uv_loop_close(loop);
        }
        if (loopKv.first != defaultId) {
            free(loop);
            loop = nullptr;
        }
    }
    loopMap.clear();
    return false;
}

void UvLoopMgr::LoopRemove(const int &id)
{
    if (id == defaultId) {
        return;
    }
    std::lock_guard<std::mutex> lock(loopMutex);
    if (loopMap.count(id) == 0) {
        return;
    }
    uv_loop_close(loopMap.at(id));
    free(loopMap.at(id));
    loopMap.erase(id);
}