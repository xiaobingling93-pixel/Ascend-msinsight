/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#ifndef PROFILER_SERVER_OPERATORGROUPCONVERTER_H
#define PROFILER_SERVER_OPERATORGROUPCONVERTER_H

#include <string>
#include <map>

namespace Dic::Protocol {
class OperatorGroupConverter {
public:
    enum class OperatorGroup {
        OP_TYPE_GROUP,
        OP_NAME_GROUP,
        OP_INPUT_SHAPE_GROUP,
        COMMUNICATION_TYPE_GROUP,
        COMMUNICATION_NAME_GROUP,
        UNKNOWN
    };

    struct OperatorGroupInfo {
        OperatorGroup group;
        bool hccl;

        OperatorGroupInfo(OperatorGroup t, bool f) : group(t), hccl(f) {}
    };

    OperatorGroupConverter()
    {
    }

    static OperatorGroup ToEnum(const std::string &type)
    {
        InitTypeMap();
        std::map<std::string, OperatorGroupInfo>::const_iterator it = typeMap.find(type);
        if (it != typeMap.end()) {
            return it->second.group;
        }
        return OperatorGroup::UNKNOWN;
    }

    static bool IsCommunication(const std::string &type)
    {
        InitTypeMap();
        std::map<std::string, OperatorGroupInfo>::const_iterator it = typeMap.find(type);
        if (it != typeMap.end()) {
            return it->second.hccl;
        }
        // 未知场景默认非HCCL
        return false;
    }

private:
    static std::map<std::string, OperatorGroupInfo> typeMap;

    static void InitTypeMap()
    {
        typeMap = { { "Operator Type", OperatorGroupInfo(OperatorGroup::OP_TYPE_GROUP, false) },
                    { "Operator", OperatorGroupInfo(OperatorGroup::OP_NAME_GROUP, false) },
                    { "Input Shape", OperatorGroupInfo(OperatorGroup::OP_INPUT_SHAPE_GROUP, false) },
                    { "Communication Operator Type", OperatorGroupInfo(OperatorGroup::COMMUNICATION_TYPE_GROUP, true) },
                    { "Communication Operator", OperatorGroupInfo(OperatorGroup::COMMUNICATION_NAME_GROUP, true) } };
    }
};
}
#endif // PROFILER_SERVER_OPERATORGROUPCONVERTER_H