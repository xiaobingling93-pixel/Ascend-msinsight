#ifndef PROFILER_SERVER_QUERY_TRITON_BASIC_INFO_HANDLER_H
#define PROFILER_SERVER_QUERY_TRITON_BASIC_INFO_HANDLER_H
#include "TritonRequestHandler.h"
namespace Dic::Module::Triton {
class QueryTritonBasicInfoHandler : public TritonRequestHandler {
public:
    QueryTritonBasicInfoHandler() { command = Protocol::REQ_RES_TRITON_MEMORY_BASIC_INFO; }
    ~QueryTritonBasicInfoHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
}
#endif
