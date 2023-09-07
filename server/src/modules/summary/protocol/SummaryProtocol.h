/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_SUMMARY_PROTOCOL_H
#define PROFILER_SERVER_SUMMARY_PROTOCOL_H

#include "ProtocolUtil.h"

namespace Dic {
namespace Protocol {
class SummaryProtocol : public ProtocolUtil {
public:
    SummaryProtocol() = default;
    ~SummaryProtocol() override = default;

private:
    void RegisterJsonToRequestFuncs() override;
    void RegisterResponseToJsonFuncs() override;
    void RegisterEventToJsonFuncs() override {};
};

} // end of namespace Protocol
} // end of namespace Dic

#endif // PROFILER_SERVER_SUMMARY_PROTOCOL_H
