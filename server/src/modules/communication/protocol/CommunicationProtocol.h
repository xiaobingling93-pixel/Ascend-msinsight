/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_COMMUNICATION_PROTOCOL_H
#define PROFILER_SERVER_COMMUNICATION_PROTOCOL_H

#include "ProtocolMessage.h"

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
    static std::unique_ptr<Request> ToIterationsRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToDurationRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToRanksRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToOperatorNamesRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToMatrixOpNamesRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToMatrixGroupRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToMatrixListRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToCommunicationAdvisorRequest(const json_t &json, std::string &error);

    static std::optional<document_t> ToOperatorDetailsResponse(const Response &response);
    static std::optional<document_t> ToBandwidthDataResponse(const Response &response);
    static std::optional<document_t> ToDistributionResponse(const Response &response);
    static std::optional<document_t> ToIterationsResponse(const Response &response);
    static std::optional<document_t> ToOperatorNamesResponse(const Response &response);
    static std::optional<document_t> ToMatrixOpNamesResponse(const Response &response);
    static std::optional<document_t> ToDurationResponse(const Response &response);
    static std::optional<document_t> ToOperatorListResponse(const Response &response);
    static std::optional<document_t> ToRanksResponse(const Response &response);
    static std::optional<document_t> ToMatrixGroupResponse(const Response &response);
    static std::optional<document_t> ToMatrixListResponse(const Response &response);
    static std::optional<document_t> ToCommunicationAdvisorResponse(const Response &response);
    // response to json
};
} // namespace Protocol
} // namespace Dic

#endif // PROFILER_SERVER_COMMUNICATION_PROTOCOL_H
