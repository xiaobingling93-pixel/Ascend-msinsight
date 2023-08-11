/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_SCENE_HARMONY_HDC_LIST_DEVICE_HANDLER_H
#define DATA_INSIGHT_CORE_SCENE_HARMONY_HDC_LIST_DEVICE_HANDLER_H

#include "HdcHandler.h"
namespace Dic {
namespace Scene {
class HdcListDeviceHandler : public HdcHandler {
public:
    HdcListDeviceHandler()
    {
        command = Protocol::REQ_RES_HDC_DEVICE_LIST;
    };
    ~HdcListDeviceHandler() override = default;
    void HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // end of namespace Scene
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_SCENE_HARMONY_HDC_LIST_DEVICE_HANDLER_H
