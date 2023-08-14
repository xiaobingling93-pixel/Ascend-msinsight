/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol declaration
 */

#ifndef DIC_PROTOCOL_ENTITY_H
#define DIC_PROTOCOL_ENTITY_H

#include <string>
#include <optional>
#include <utility>
#include <vector>
#include <memory>
#include <map>
#include "ProtocolEnum.h"

namespace Dic {
namespace Protocol {
// harmony
struct Device {
    std::string deviceKey;
    DeviceStatus status = DeviceStatus::OFFLINE;
    DeviceConnectType connectType = DeviceConnectType::USB;
    std::string cpuAbi;
    int apiVersion = 0;
    std::string productModel;
    std::string deviceType;
    std::string softwareVersion;
    std::string productBrand;
};

struct Process {
    int pid = 0;
    int ppid = 0;
    std::string processName;
    std::optional<std::string> bundleName;
    ProcessStatus status = ProcessStatus::ALIVE;
    std::optional<std::string> uid;
    ProcessType type = ProcessType::SYSTEM_PROCESS;
    bool isDebug = false;
};

struct Application {
    std::string bundleName;
    ApplicationStatus status = ApplicationStatus::INSTALLED;
};

// global
struct GlobalConfig {
    int maxSessionCount = 20;
};
// ascend
struct AscendConfig {
    int maxSessionCount = 20;
};
// harmony
struct HdcConfig {
    bool enable = true;
    std::string path;
    std::string traceDir;
    int hdcPort = 0;
};

struct DfxConfig {
    bool enable = true;
    std::string dbDir;
};

struct JsVmConfig {
    bool enable = true;
};

struct HarmonyConfig {
    HdcConfig hdc;
    DfxConfig dfx;
    JsVmConfig jsvm;
};
} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_PROTOCOL_ENTITY_H
