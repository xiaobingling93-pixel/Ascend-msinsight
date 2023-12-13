/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_PROTOCOL_MANAGER_H
#define PROFILER_SERVER_PROTOCOL_MANAGER_H


#include <string>
#include <map>
#include <functional>
#include <mutex>
#include <optional>
#include <memory>
#include "GlobalDefs.h"
#include "ProtocolMessage.h"
#include "ProtocolUtil.h"

namespace Dic {
namespace Protocol {
class ProtocolManager {
public:
    static ProtocolManager &Instance()
    {
        static ProtocolManager instance;
        return instance;
    }

    std::unique_ptr<Request> FromJson(const std::string &requestStr, std::string &error);
    std::optional<document_t> ToJson(const Response &response, std::string &error);
    std::optional<document_t> ToJson(const Event &event, std::string &error);

private:
    ProtocolManager();
    ~ProtocolManager();

    void Register();
    void UnRegister();

    std::mutex mutex;
    std::map<Dic::Protocol::ModuleType, std::unique_ptr<ProtocolUtil>> protocolMap;
};
} // namespace Protocol
} // namespace Dic
#endif // PROFILER_SERVER_PROTOCOL_MANAGER_H
