/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_SUMMARY_PROTOCOL_H
#define PROFILER_SERVER_SUMMARY_PROTOCOL_H

#include "ProtocolMessage.h"

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
    static std::unique_ptr<Request> ToComputeDetailRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToStepRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToStagesRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToStageTimeRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToRankTimeRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToCommunicationRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToQueryParallelStrategyRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToSetParallelStrategyRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToQueryFwdBwdTimelineRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToQueryParallelismArrangementRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToQueryParallelismPerformanceRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToImportExpertDataRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToQueryExpertHotspotRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToQueryModelInfoRequest(const json_t &json, std::string &error);

    // response to json
    static std::optional<document_t> ToTopNResponse(const Response &response);
    static std::optional<document_t> ToStatisticsResponse(const Response &response);
    static std::optional<document_t> ToComputeDetailResponse(const Response &response);
    static std::optional<document_t> ToStepResponse(const Response &response);
    static std::optional<document_t> ToStagesResponse(const Response &response);
    static std::optional<document_t> ToStageTimeResponse(const Response &response);
    static std::optional<document_t> ToRankTimeResponse(const Response &response);
    static std::optional<document_t> ToCommunicationResponse(const Response &response);
    static std::optional<document_t> ToQueryParallelStrategyResponse(const Response &response);
    static std::optional<document_t> ToSetParallelStrategyResponse(const Response &response);
    static std::optional<document_t> ToQueryFwdBwdTimelineResponse(const Response &response);
    static std::optional<document_t> ToQueryParallelismArrangementResponse(const Response &response);
    static std::optional<document_t> ToQueryParallelismPerformanceResponse(const Response &response);
    static std::optional<document_t> ToImportExpertDataResponse(const Response &response);
    static std::optional<document_t> ToQueryExpertHotspotResponse(const Response &response);
    static std::optional<document_t> ToQueryModelInfoResponse(const Response &response);
};
} // namespace Protocol
} // namespace Dic

#endif // PROFILER_SERVER_SUMMARY_PROTOCOL_H
