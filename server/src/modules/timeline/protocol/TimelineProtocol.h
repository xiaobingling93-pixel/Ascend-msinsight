/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_TIMELINE_PROTOCOL_H
#define PROFILER_SERVER_TIMELINE_PROTOCOL_H

#include "ProtocolMessage.h"

namespace Dic {
namespace Protocol {
class TimelineProtocol : public ProtocolUtil {
public:
    TimelineProtocol() = default;
    ~TimelineProtocol() override = default;
    static std::optional<document_t> ToModuleResetEventJson(const Event &event);
    static std::optional<document_t> ToAllSuccessEventJson(const Event &event);

private:
    void RegisterJsonToRequestFuncs() override;
    void RegisterResponseToJsonFuncs() override;
    void RegisterEventToJsonFuncs() override;

    // json to request
    static std::unique_ptr<Request> ToImportActionRequest(const json_t &json, std::string &error);
    // json to request
    static std::unique_ptr<Request> ToParseCardsRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToUnitThreadTracesRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToUnitThreadTracesSummaryRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToUnitThreadsRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToThreadDetailRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToUnitFlowsRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToResetWindowRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToSearchCountRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToSearchSliceRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToRemoteDeleteRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToFlowCategoryListRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToFlowCategoryEventsRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToUnitCounterRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToSystemViewRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToEventsViewRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToKernelDetailRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToOneKernelRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToUnitThreadsOperatorsRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToSearchAllSlicesRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToCommunicationKernelRequest(const Dic::json_t &json, std::string &error);
    static std::unique_ptr<Request> ToSystemViewOverallRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToSystemViewOverallMoreDetailsRequest(const json_t &json, std::string &error);
    // response to json
    static std::optional<document_t> ToImportActionResponseJson(const Response &response);
    static std::optional<document_t> ToUnitThreadTracesResponseJson(const Response &response);
    static std::optional<document_t> ToUnitThreadTracesSummaryResponseJson(const Response &response);
    static std::optional<document_t> ToUnitThreadsResponseJson(const Response &response);
    static std::optional<document_t> ToThreadDetailResponseJson(const Response &response);
    static std::optional<document_t> ToUnitFlowsResponseJson(const Response &response);
    static std::optional<document_t> ToResetWindowResponseJson(const Response &response);
    static std::optional<document_t> ToSearchCountResponseJson(const Response &response);
    static std::optional<document_t> ToSearchSliceResponseJson(const Response &response);
    static std::optional<document_t> ToRemoteDeleteResponseJson(const Response &response);
    static std::optional<document_t> ToFlowCategoryListResponse(const Response &response);
    static std::optional<document_t> ToFlowCategoryEventsResponse(const Response &response);
    static std::optional<document_t> ToUnitCounterResponse(const Response &response);
    static std::optional<document_t> ToSystemViewResponseJson(const Response &response);
    static std::optional<document_t> ToEventsViewResponseJson(const Response &response);
    static std::optional<document_t> ToKernelDetailResponseJson(const Response &response);
    static std::optional<document_t> ToOneKernelResponseJson(const Response &response);
    static std::optional<document_t> ToCommunicationKernelResponseJson(const Dic::Protocol::Response &response);
    static std::optional<document_t> ToUnitThreadsOperatorsResponseJson(const Response &response);
    static std::optional<document_t> ToSearchAllSlicesResponseJson(const Response &response);
    static std::optional<document_t> ToParseCardsResponseJson(const Response &response);
    // event to json
    static std::optional<document_t> ToParseSuccessEventJson(const Event &event);
    static std::optional<document_t> ToParseFailEventJson(const Event &event);
    static std::optional<document_t> ToParseClusterCompletedEventJson(const Event &event);
    static std::optional<document_t> ToParseClusterStep2CompletedEventJson(const Event &event);
    static std::optional<document_t> ToParseMemoryCompletedEventJson(const Event &event);
    static std::optional<document_t> ToParseProgressEventJson(const Event &event);
    static std::optional<document_t> ToSystemViewOverallResponseJson(const Response &response);
    static std::optional<document_t> ToOverallMoreDetailsResponseJson(const Response &response);
};
} // namespace Protocol
} // namespace Dic

#endif // PROFILER_SERVER_TIMELINE_PROTOCOL_H
