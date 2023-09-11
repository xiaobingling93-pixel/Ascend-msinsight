/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_COMMUNICATORGROUPPARSER_H
#define PROFILER_SERVER_COMMUNICATORGROUPPARSER_H

#include <string>
#include "ProtocolMessage.h"
#include "ProtocolDefs.h"
#include "CommunicationRequestHandler.h"
#include "CommunicationProtocolResponse.h"

namespace Dic {
namespace Module {
namespace Communication {
class CommunicatorGroupParserHandler : public CommunicationRequestHandler {
public:
    CommunicatorGroupParserHandler()
    {
        command = Protocol::REQ_RES_COMMUNICATOR_PARSE;
    };
    static bool ParseCommunicatorGroup(const std::string &, Dic::Protocol::CommunicatorGroupResBody &resBody);
    void HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // end of namespace Communication
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_COMMUNICATORGROUPPARSER_H
