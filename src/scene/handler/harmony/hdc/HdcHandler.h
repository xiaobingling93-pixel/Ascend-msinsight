/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_SCENE_HARMONY_HDC_HANDLER_H
#define DATA_INSIGHT_CORE_SCENE_HARMONY_HDC_HANDLER_H

#include "HarmonyHandler.h"

namespace Dic {
namespace Scene {
class HdcHandler : public HarmonyHandler {
public:
    HdcHandler()
    {
        scope = "hdc";
    }
    ~HdcHandler() override = default;
    void HandleRequest(std::unique_ptr<Request> requestPtr) override {};
};
} // end of namespace Scene
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_SCENE_HARMONY_HDC_HANDLER_H