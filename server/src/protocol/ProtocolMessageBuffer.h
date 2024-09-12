/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: declaration for request message buffer
 */

#ifndef DATA_INSIGHT_CORE_SERVER_MESSAGE_BUFFER_H
#define DATA_INSIGHT_CORE_SERVER_MESSAGE_BUFFER_H

#include <mutex>
#include <memory>
#include <limits>
#include "ProtocolMessage.h"

namespace Dic {
namespace Protocol {
class ProtocolMessageBuffer {
public:
    ProtocolMessageBuffer() = default;
    ~ProtocolMessageBuffer() = default;

    ProtocolMessageBuffer &operator << (const std::string &data);
    std::unique_ptr<Protocol::ProtocolMessage> Pop();
    void Clear();

private:
    uint64_t GetBodyLength(const uint64_t &headPosition, const uint64_t &headLength) const;
    Protocol::ProtocolMessage::Type GetMessageType(const std::string &body) const;

    const std::string REQ_DELIMITER = "\r\n\r\n";
    const std::string HEAD_START = "Content-Length:";
    const uint32_t matchMinNum = 2;
    const uint64_t bufferLimit = 16 * 1024 * 1024;
    const uint64_t invalidBodyLen = std::numeric_limits<uint64_t>::max();
    std::mutex mutex;
    std::string buffer;
    std::string error;
    uint64_t headPos = 0;
    uint64_t headLen = 0;
    uint64_t bodyPos = 0;
    uint64_t bodyLen = 0;
};
} // end of namespace Protocol
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_SERVER_MESSAGE_BUFFER_H
