/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_WS_DEFS_H
#define DATA_INSIGHT_CORE_WS_DEFS_H

#include <map>
#include <string>

namespace Dic {
// websocket close code & reason
static const unsigned short URL_NULL_CODE = 1;
static const unsigned short SID_UN_CORRECT_CODE = 2;
static const std::map<unsigned short, std::string> WS_CLOSE_CODE_REASON = {
    { URL_NULL_CODE, "url is null" },
    { SID_UN_CORRECT_CODE, "sid is not correct" },
    { 1000, "Connection was closed normally." },
    { 1001, "An endpoint is going away." },
    { 1002, "An endpoint is terminating the connection due to a protocol error." },
    { 1003, "An endpoint is terminating the connection due to that data cannot be accept." },
    { 1004, "Reserved." },
    { 1005, "No status code was actually present." },
    { 1006, "Connection was closed abnormally." },
    { 1007, "An endpoint is terminating the connection due to wrong message type." },
    { 1008, "An endpoint is terminating the connection due to a message violates its policy." },
    { 1009, "An endpoint is terminating the connection due to an big message." },
    { 1010, "Client is terminating the connection due to an unexpected negotiation with server." },
    { 1011, "Server is terminating the connection due to an unexpected condition." },
    { 1012, "Reserved." },
    { 1013, "Reserved." },
    { 1014, "Reserved." },
    { 1015, "Closed due to a failure to perform a TLS handshake." }
};

static inline std::string GetCloseReason(unsigned short code)
{
    if (WS_CLOSE_CODE_REASON.count(code) == 0) {
        return "Unknown reason.";
    }
    return WS_CLOSE_CODE_REASON.at(code);
}
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_WS_DEFS_H
