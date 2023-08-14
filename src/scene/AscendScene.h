//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
//

#ifndef PROFILER_SERVER_ASCEND_SCENE_H
#define PROFILER_SERVER_ASCEND_SCENE_H

#include "BaseScene.h"

namespace Dic {
namespace Scene {
class AscendScene : public BaseScene {
public:
    AscendScene();
    ~AscendScene() override;

    void Config(const AscendConfig &cfg);
    const AscendConfig &GetConfig() const;
    void RegisterRequestHandlers() override;
    void OnRequest(std::unique_ptr<Protocol::Request> request) override;

private:
    AscendConfig config;
};
} // end of namespace Scene
} // end of namespace Dic


#endif // PROFILER_SERVER_ASCEND_SCENE_H
