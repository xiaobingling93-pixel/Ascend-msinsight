/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_COMMUNICATION_PROTOCOL_H
#define PROFILER_SERVER_COMMUNICATION_PROTOCOL_H

#include "ProtocolUtil.h"

namespace Dic {
namespace Protocol {
class CommunicationProtocol : public ProtocolUtil {
public:
    CommunicationProtocol() = default;
    ~CommunicationProtocol() override = default;

private:
    void RegisterJsonToRequestFuncs() override;
    void RegisterResponseToJsonFuncs() override;
    void RegisterEventToJsonFuncs() override;

    // json to request
    static std::unique_ptr<Request> ToOperatorDetailsRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToBandwidthDataRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToDistributionRequest(const json_t &json, std::string &error);

    static std::optional<json_t> ToOperatorDetailsResponse(const Response &response);
    static std::optional<json_t> ToBandwidthDataResponse(const Response &response);
    static std::optional<json_t> ToDistributionResponse(const Response &response);
    // response to json
};
} // namespace Protocol
} // namespace Dic

#endif // PROFILER_SERVER_COMMUNICATION_PROTOCOL_H
