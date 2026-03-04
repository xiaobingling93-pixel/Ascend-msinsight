#include "QueryTritonMemoryUsageHandler.h"
#include "TritonProtocolRequest.h"
#include "TritonProtocolResponse.h"
#include "TritonService.h"
#include "WsSender.h"

namespace Dic::Module::Triton {
bool QueryTritonMemoryUsageHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    const auto& request = dynamic_cast<Protocol::TritonMemoryUsageRequest &>(*requestPtr);
    auto responsePtr = std::make_unique<Protocol::TritonMemoryUsageResponse>();
    auto& response = *responsePtr;
    SetBaseResponse(request, response);
    const auto& service = TritonService::Instance();
    auto segments = service.QuerySegmentsContainRange(request.timestamp);
    response.segments = std::move(segments);
    SendResponse(std::move(responsePtr), true);
    return true;
}
}
