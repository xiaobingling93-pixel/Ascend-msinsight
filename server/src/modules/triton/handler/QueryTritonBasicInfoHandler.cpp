#include "QueryTritonBasicInfoHandler.h"
#include "TritonProtocolRequest.h"
#include "TritonProtocolResponse.h"
#include "TritonService.h"
#include "WsSender.h"
namespace Dic::Module::Triton {
bool QueryTritonBasicInfoHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    const auto& request = dynamic_cast<Protocol::TritonBasicInfoRequest&>(*requestPtr);
    std::unique_ptr<Protocol::TritonBasicInfoResponse> responsePtr = std::make_unique<Protocol::TritonBasicInfoResponse>();
    auto& response = *responsePtr;
    SetBaseResponse(request, response);
    auto [kernelName] = TritonService::Instance().GetHeader();
    response.kernelName = kernelName;
    SendResponse(std::move(responsePtr), true);
    return true;
}
}
