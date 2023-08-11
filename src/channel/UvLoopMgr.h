/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: declaration of UvLoopMgr
 */
#ifndef DIC_UV_LOOP_MGR_H
#define DIC_UV_LOOP_MGR_H

#include <map>
#include <mutex>
#include <uv.h>

namespace Dic {
class UvLoopMgr {
public:
    static UvLoopMgr &Instance();

    uv_loop_t *Loop(const int &id = -1);
    bool LoopInit(const int &id = -1);
    void LoopStart(const int &id = -1);
    void LoopStop(const int &id = -1);
    void LoopRemove(const int &id = -1); // internal api
    bool AllLoopStop();

private:
    UvLoopMgr();
    ~UvLoopMgr();

    int defaultId;
    std::mutex loopMutex;
    std::map<int, uv_loop_t *> loopMap;
};
}

#endif // DIC_UV_LOOP_MGR_H
