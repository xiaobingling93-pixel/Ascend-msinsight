/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol response implementation
 */

#include "ServerLog.h"
#include "JsonUtil.h"
#include "ProtocolEnumUtil.h"
#include "ProtocolUtil.h"
#include "ResponseUtil.h"
#include "ResponseManager.h"

namespace Dic {
namespace Protocol {
using namespace Dic::Server;

std::optional<ResponseManager::ResponseToJsonFunc> ResponseManager::GetResponseToJsonFunc(const std::string &command)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (resToJsonFactory.count(command) == 0) {
        ServerLog::Warn("The response to json function is not find. command:", command);
        return std::nullopt;
    }
    return resToJsonFactory[command];
}

/*
 * 将Response转为json格式
 *
 * response：待转换的请求
 * error：转换过程中的错误信息
 * @return 转换后的结果
 */
std::optional<json_t> ResponseManager::ToJson(const Response &response, std::string &error)
{
    std::string command = response.command;
    std::optional<ResponseManager::ResponseToJsonFunc> func = GetResponseToJsonFunc(command);
    if (!func.has_value()) {
        error = "Failed to find response target function, token = " + response.token + ", command = " + command;
        return std::nullopt;
    }
    try {
        return func.value()(response);
    } catch (std::exception &e) {
        error = "Failed to convert response to json, command = " + command;
        return std::nullopt;
    }
}

ResponseManager::ResponseManager()
{
    Register();
}

ResponseManager::~ResponseManager()
{
    UnRegister();
}

void ResponseManager::Register()
{
    std::lock_guard<std::mutex> lock(mutex);
    RegisterResponseToJsonFuncs();
}

void ResponseManager::RegisterResponseToJsonFuncs()
{
    // global
    resToJsonFactory.emplace(REQ_RES_TOKEN_CREATE, ToTokenCreateResponseJson);
    resToJsonFactory.emplace(REQ_RES_TOKEN_DESTROY, ToTokenDestroyResponseJson);
    resToJsonFactory.emplace(REQ_RES_TOKEN_CHECK, ToTokenCheckResponseJson);
    resToJsonFactory.emplace(REQ_RES_CONFIG_GET, ToConfigGetResponseJson);
    resToJsonFactory.emplace(REQ_RES_CONFIG_SET, ToConfigSetResponseJson);
    // timeline
    resToJsonFactory.emplace(REQ_RES_IMPORT_ACTION, ToImportActionResponseJson);
    resToJsonFactory.emplace(REQ_RES_IMPORT_ACTION, ToImportActionResponseJson);
    resToJsonFactory.emplace(REQ_RES_UNIT_THREAD_TRACES, ToUnitThreadTracesResponseJson);
    resToJsonFactory.emplace(REQ_RES_UNIT_THREADS, ToUnitThreadsResponseJson);
    resToJsonFactory.emplace(REQ_RES_UNIT_THREAD_DETAIL, ToThreadDetailResponseJson);
    resToJsonFactory.emplace(REQ_RES_UNIT_FLOW_NAME, ToUnitFlowNameResponseJson);
    resToJsonFactory.emplace(REQ_RES_UNIT_FLOW, ToUnitFlowResponseJson);
    resToJsonFactory.emplace(REQ_RES_RESET_WINDOW, ToResetWindowResponseJson);
    resToJsonFactory.emplace(REQ_RES_UNIT_CHART, ToUnitChartResponseJson);
}

void ResponseManager::UnRegister()
{
    std::lock_guard<std::mutex> lock(mutex);
    resToJsonFactory.clear();
}

#pragma region << Response To Json>>
// global
std::optional<json_t> ResponseManager::ToTokenCreateResponseJson(const Response &response)
{
    return ToResponseJson<TokenCreateResponse>(dynamic_cast<const TokenCreateResponse &>(response));
}

std::optional<json_t> ResponseManager::ToTokenDestroyResponseJson(const Response &response)
{
    return ToResponseJson<TokenDestroyResponse>(dynamic_cast<const TokenDestroyResponse &>(response));
}

std::optional<json_t> ResponseManager::ToTokenCheckResponseJson(const Response &response)
{
    return ToResponseJson<TokenCheckResponse>(dynamic_cast<const TokenCheckResponse &>(response));
}

std::optional<json_t> ResponseManager::ToConfigGetResponseJson(const Response &response)
{
    return ToResponseJson<ConfigGetResponse>(dynamic_cast<const ConfigGetResponse &>(response));
}

std::optional<json_t> ResponseManager::ToConfigSetResponseJson(const Response &response)
{
    return ToResponseJson<ConfigSetResponse>(dynamic_cast<const ConfigSetResponse &>(response));
}

std::optional<json_t> ResponseManager::ToImportActionResponseJson(const Response &response)
{
    return ToResponseJson<ImportActionResponse>(dynamic_cast<const ImportActionResponse &>(response));
}

std::optional<json_t> ResponseManager::ToUnitThreadTracesResponseJson(const Response &response)
{
    return ToResponseJson<UnitThreadTracesResponse>(dynamic_cast<const UnitThreadTracesResponse &>(response));
}

std::optional<json_t> ResponseManager::ToUnitThreadsResponseJson(const Response &response)
{
    return ToResponseJson<UnitThreadsResponse>(dynamic_cast<const UnitThreadsResponse &>(response));
}

std::optional<json_t> ResponseManager::ToThreadDetailResponseJson(const Response &response)
{
    return ToResponseJson<UnitThreadDetailResponse>(dynamic_cast<const UnitThreadDetailResponse &>(response));
}

std::optional<json_t> ResponseManager::ToUnitFlowNameResponseJson(const Response &response)
{
    return ToResponseJson<UnitFlowNameResponse>(dynamic_cast<const UnitFlowNameResponse &>(response));
}

std::optional<json_t> ResponseManager::ToUnitFlowResponseJson(const Response &response)
{
    return ToResponseJson<UnitFlowResponse>(dynamic_cast<const UnitFlowResponse &>(response));
}

std::optional<json_t> ResponseManager::ToResetWindowResponseJson(const Response &response)
{
    return ToResponseJson<ResetWindowResponse>(dynamic_cast<const ResetWindowResponse &>(response));
}

std::optional<json_t> ResponseManager::ToUnitChartResponseJson(const Response &response)
{
    return ToResponseJson<UnitChartResponse>(dynamic_cast<const UnitChartResponse &>(response));
}

#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic