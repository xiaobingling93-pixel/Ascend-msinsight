/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_RLMICROBATCHCLASSIFIERFACTORY_H
#define PROFILER_SERVER_RLMICROBATCHCLASSIFIERFACTORY_H
#include <memory>
#include "RLDomainObject.h"
#include "RLMicroBatchMegatronClassifier.h"
#include "RLMicroBatchFSDPClassifier.h"
namespace Dic::Module::RL {
class RLMicroBatchClassifierFactory {
public:
    static std::shared_ptr<RLMicroBatchClassifierBase> GetClassifier(RLBackEndType type)
    {
        switch (type) {
            case RLBackEndType::Megatron:
                return std::make_shared<RLMicroBatchMegatronClassifier>();
            case RLBackEndType::FSDP:
                return std::make_shared<RLMicroBatchFSDPClassifier>();
            case RLBackEndType::Unknown:
            default:
                return nullptr;
        }
    }
};
}

#endif // PROFILER_SERVER_RLMICROBATCHCLASSIFIERFACTORY_H
