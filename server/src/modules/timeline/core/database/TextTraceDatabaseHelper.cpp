/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "TextTraceDatabaseHelper.h"
#include "ServerLog.h"
#include "NumberUtil.h"
#include "JsonUtil.h"

namespace Dic::Module::Timeline {
using namespace Server;

void TextTraceDatabaseHelper::ProcessByteAlignmentAnalyzerDataForText(
    std::vector<CommunicationLargeOperatorInfo> &result, std::vector<std::pair<std::string, std::string>> rawData)
{
    bool hasOneHcom = false;
    CommunicationLargeOperatorInfo op;
    for (const auto &item : rawData) {
        if (item.first.find("hcom") == 0) {
            if (hasOneHcom) {
                result.push_back(op);
            } else {
                hasOneHcom = true;
            }
            op.name = item.first;
            op.memcpyTasks.clear();
            op.reduceInlineTasks.clear();
        } else {
            if (!hasOneHcom) {
                continue;
            }
            std::string err;
            std::optional<document_t> jsonOptional = JsonUtil::TryParse(item.second, err);
            if (jsonOptional == std::nullopt) {
                ServerLog::Error("Failed to parse args. ", err);
                continue;
            }
            document_t &json = jsonOptional.value();
            if (!json.IsObject()) {
                ServerLog::Error("Args is not valid json format. raw: %", item.second);
                continue;
            }
            CommunicationSmallOperatorInfo info;
            int64_t tempSize = NumberUtil::StringToLongLong(JsonUtil::GetString(json, "size(Byte)"));
            info.size = (tempSize < 0 ? 0 : static_cast<uint64_t>(tempSize));
            info.transportType = JsonUtil::GetString(json, "transport type");
            info.linkType = JsonUtil::GetString(json, "link type");
            if (item.first.find("Memcpy") == 0) {
                op.memcpyTasks.emplace_back(info);
            } else {
                op.reduceInlineTasks.emplace_back(info);
            }
        }
    }
    result.push_back(op);
}
}