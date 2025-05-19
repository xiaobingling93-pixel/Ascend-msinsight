/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */


#ifndef PROFILER_SERVER_ADVISORPROTOCOLFROMREQUESTJSON_H
#define PROFILER_SERVER_ADVISORPROTOCOLFROMREQUESTJSON_H

#include "GlobalDefs.h"
#include "AdvisorProtocolRequest.h"

namespace Dic::Protocol {
template<typename RequestType> std::unique_ptr<Request> ToRequest(const Dic::json_t& json, std::string& error);
class AdvisorProtocolFromRequestJson {
public:
static std::unique_ptr<Request> ToAffinityOptimizerRequest(const json_t &json, std::string &error);
static std::unique_ptr<Request> ToAffinityAPIRequest(const json_t &json, std::string &error);
static std::unique_ptr<Request> ToOperatorFusionRequest(const json_t &json, std::string &error);
static std::unique_ptr<Request> ToAICpuOperatorRequest(const json_t &json, std::string &error);
static std::unique_ptr<Request> ToAclnnOperatorRequest(const json_t &json, std::string &error);
static std::unique_ptr<Request> ToOperatorDispatchRequest(const json_t &json, std::string &error);
};
}

#endif // PROFILER_SERVER_ADVISORPROTOCOLFROMREQUESTJSON_H
