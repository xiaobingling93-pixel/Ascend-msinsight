/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_TIMELINE_PROTOCOL_H
#define PROFILER_SERVER_TIMELINE_PROTOCOL_H

#include "ProtocolUtil.h"

namespace Dic {
namespace Protocol {
class TimelineProtocol : public ProtocolUtil {
public:
    TimelineProtocol() = default;
    ~TimelineProtocol() override = default;

private:
    void RegisterJsonToRequestFuncs() override;
    void RegisterResponseToJsonFuncs() override;
    void RegisterEventToJsonFuncs() override;

    // json to request
    static std::unique_ptr<Request> ToImportActionRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToUnitThreadTracesRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToUnitThreadsRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToThreadDetailRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToUnitFlowNameRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToUnitFlowRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToResetWindowRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToUnitChartRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToSearchCountRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToSearchSliceRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToRemoteDeleteRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToFlowCategoryListRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToFlowCategoryEventsRequest(const json_t &json, std::string &error);
    static std::unique_ptr<Request> ToUnitCounterRequest(const json_t &json, std::string &error);
    // response to json
    static std::optional<json_t> ToImportActionResponseJson(const Response &response);
    static std::optional<json_t> ToUnitThreadTracesResponseJson(const Response &response);
    static std::optional<json_t> ToUnitThreadsResponseJson(const Response &response);
    static std::optional<json_t> ToThreadDetailResponseJson(const Response &response);
    static std::optional<json_t> ToUnitFlowNameResponseJson(const Response &response);
    static std::optional<json_t> ToUnitFlowResponseJson(const Response &response);
    static std::optional<json_t> ToResetWindowResponseJson(const Response &response);
    static std::optional<json_t> ToUnitChartResponseJson(const Response &response);
    static std::optional<json_t> ToSearchCountResponseJson(const Response &response);
    static std::optional<json_t> ToSearchSliceResponseJson(const Response &response);
    static std::optional<json_t> ToRemoteDeleteResponseJson(const Response &response);
    static std::optional<json_t> ToFlowCategoryListResponse(const Response &response);
    static std::optional<json_t> ToFlowCategoryEventsResponse(const Response &response);
    static std::optional<json_t> ToUnitCounterResponse(const Response &response);
    // event to json
    static std::optional<json_t> ToParseSuccessEventJson(const Event &event);
    static std::optional<json_t> ToParseFailEventJson(const Event &event);
    static std::optional<json_t> ToParseClusterCompletedEventJson(const Event &event);
    static std::optional<json_t> ToParseMemoryCompletedEventJson(const Event &event);
};
} // namespace Protocol
} // namespace Dic

#endif // PROFILER_SERVER_TIMELINE_PROTOCOL_H
