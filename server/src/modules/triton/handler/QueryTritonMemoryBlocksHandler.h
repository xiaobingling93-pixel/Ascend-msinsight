#ifndef PROFILER_SERVER_QUERY_TRITON_MEMORY_BLOCKS_HANDLER_H
#define PROFILER_SERVER_QUERY_TRITON_MEMORY_BLOCKS_HANDLER_H
#include "TritonRequestHandler.h"
namespace Dic::Module::Triton {
class QueryTritonMemoryBlocksHandler : public TritonRequestHandler {
public:
    QueryTritonMemoryBlocksHandler() { command = Protocol::REQ_RES_TRITON_MEMORY_BLOCKS; }
    ~QueryTritonMemoryBlocksHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
}
#endif
