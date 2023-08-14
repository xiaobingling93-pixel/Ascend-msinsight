//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
//

#ifndef PROFILER_SERVER_QUERY_IMPORT_DATA_HANDLER_H
#define PROFILER_SERVER_QUERY_IMPORT_DATA_HANDLER_H

#include "AscendRequestHandler.h"

namespace Dic {
namespace Scene {
class QueryImportDataHandler : public AscendRequestHandler {
public:
    QueryImportDataHandler()
    {
        command = Protocol::REQ_RES_IMPORT_ACTION;
    };
    ~QueryImportDataHandler() override = default;
    void HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // end of namespace Scene
} // end of namespace Dic


#endif // PROFILER_SERVER_QUERY_IMPORT_DATA_HANDLER_H
