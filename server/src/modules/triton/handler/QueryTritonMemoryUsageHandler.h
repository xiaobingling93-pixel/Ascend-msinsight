#ifndef PROFILER_SERVER_QUERY_TRITON_MEMORY_USAGE_HANDLER_H
#define PROFILER_SERVER_QUERY_TRITON_MEMORY_USAGE_HANDLER_H
#include "TritonRequestHandler.h"

namespace Dic::Module::Triton {
class QueryTritonMemoryUsageHandler : public TritonRequestHandler {
public:
    QueryTritonMemoryUsageHandler()
    {
        command = Protocol::REQ_RES_TRITON_MEMORY_USAGE;
    }
    ~QueryTritonMemoryUsageHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
}
#endif
