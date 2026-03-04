#include "QueryTritonMemoryBlocksHandler.h"
#include "TritonProtocolRequest.h"
#include "TritonProtocolResponse.h"
#include "TritonService.h"
#include "WsSender.h"
namespace Dic::Module::Triton {
bool QueryTritonMemoryBlocksHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto& request = dynamic_cast<Protocol::TritonMemoryBlocksRequest&>(*requestPtr);
    std::unique_ptr<Protocol::TritonMemoryBlocksResponse> responsePtr = std::make_unique<Protocol::TritonMemoryBlocksResponse>();
    auto& response = *responsePtr;
    SetBaseResponse(request, response);
    if (request.startTimestamp > request.endTimestamp) {
        SendResponse(std::move(responsePtr), false, "Invalid range: start timestamp is greater than end time.");
        return false;
    }
    auto& service = TritonService::Instance();
    auto blocks = service.QueryBlocksContainRange(request.startTimestamp, request.endTimestamp);
    response.blocks = std::move(blocks);
    response.UpdateInfo();
    SendResponse(std::move(responsePtr), true);
    return true;
}
}
