/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
    void RegisterEventToJsonFuncs() override;

    // json to request
    static std::unique_ptr<Request> ToTopNRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToStatisticsRequest(const json_t &json, std::string &error);

    static std::optional<json_t> ToTopNResponse(const Response &response);
    static std::optional<json_t> ToStatisticsResponse(const Response &response);
    // response to json
};
} // namespace Protocol
} // namespace Dic

#endif // PROFILER_SERVER_SUMMARY_PROTOCOL_H
