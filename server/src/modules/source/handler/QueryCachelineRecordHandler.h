/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
 
#ifndef PROFILER_SERVER_QUERY_CACHELINE_RECORD_HANDLER_H
#define PROFILER_SERVER_QUERY_CACHELINE_RECORD_HANDLER_H
 
#include "ModuleRequestHandler.h"
 
namespace Dic {
namespace Module {
namespace Source {
class QueryCachelineRecordHandler : public ModuleRequestHandler {
public:
    QueryCachelineRecordHandler()
    {
        command = Protocol::REQ_RES_CACHELINE_RECORD;
    }
 
    ~QueryCachelineRecordHandler() override = default;
 
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
private:
    std::string QueryCachelineRecord(const std::string& filePath);
};
} // Source
} // Module
} // Dic
 
#endif // PROFILER_SERVER_QUERY_CACHELINE_RECORD_HANDLER_H
