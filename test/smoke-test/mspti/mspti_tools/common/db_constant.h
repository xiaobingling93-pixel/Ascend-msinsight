#include <cstddef> // offsetof
#include <cstdint>
#include <sqlite3.h>
#include <vector>
#include <string>

#include "mspti.h"


// 字段类型枚举
enum class FieldType { INTEGER, REAL, TEXT };

// 字段描述
struct FieldDef {
    std::string name;   // 字段名
    FieldType type;     // 字段类型
    size_t offset;      // 在结构体中的偏移
};

template<typename T>
struct ActivityTraits; // 每个类型的特化会提供字段映射和表名

#define REGISTER_ACTIVITY_TYPE(TYPE, TABLE, FIELDS) \
template<> struct ActivityTraits<TYPE> { \
    static inline const std::string tableName = TABLE; \
    static inline const std::vector<FieldDef> fields = FIELDS; \
}

REGISTER_ACTIVITY_TYPE(msptiActivityKernel, "kernel", {
    {"kind", FieldType::INTEGER, offsetof(msptiActivityKernel, kind)},
    {"start", FieldType::INTEGER, offsetof(msptiActivityKernel, start)},
    {"end", FieldType::INTEGER, offsetof(msptiActivityKernel, end)},
    {"deviceId", FieldType::INTEGER, offsetof(msptiActivityKernel, ds.deviceId)},
    {"streamId", FieldType::INTEGER, offsetof(msptiActivityKernel, ds.streamId)},
    {"correlationId", FieldType::INTEGER, offsetof(msptiActivityKernel, correlationId)},
    {"type", FieldType::TEXT, offsetof(msptiActivityKernel, type)},
    {"name", FieldType::TEXT, offsetof(msptiActivityKernel, name)}
});

REGISTER_ACTIVITY_TYPE(msptiActivityHccl, "hccl", {
    {"kind", FieldType::INTEGER, offsetof(msptiActivityHccl, kind)},
    {"start", FieldType::INTEGER, offsetof(msptiActivityHccl, start)},
    {"end", FieldType::INTEGER, offsetof(msptiActivityHccl, end)},
    {"deviceId", FieldType::INTEGER, offsetof(msptiActivityHccl, ds.deviceId)},
    {"streamId", FieldType::INTEGER, offsetof(msptiActivityHccl, ds.streamId)},
    {"bandWidth", FieldType::REAL, offsetof(msptiActivityHccl, bandWidth)},
    {"name", FieldType::TEXT, offsetof(msptiActivityHccl, name)},
    {"commName", FieldType::TEXT, offsetof(msptiActivityHccl, commName)}
});

REGISTER_ACTIVITY_TYPE(msptiActivityApi, "api", {
    {"kind", FieldType::INTEGER, offsetof(msptiActivityApi, kind)},
    {"start", FieldType::INTEGER, offsetof(msptiActivityApi, start)},
    {"end", FieldType::INTEGER, offsetof(msptiActivityApi, end)},
    {"processId", FieldType::INTEGER, offsetof(msptiActivityApi, pt.processId)},
    {"threadId", FieldType::INTEGER, offsetof(msptiActivityApi, pt.threadId)},
    {"correlationId", FieldType::INTEGER, offsetof(msptiActivityApi, correlationId)},
    {"name", FieldType::TEXT, offsetof(msptiActivityApi, name)}
});

REGISTER_ACTIVITY_TYPE(msptiActivityCommunication, "communication", {
    {"kind", FieldType::INTEGER, offsetof(msptiActivityCommunication, kind)},
    {"dataType", FieldType::INTEGER, offsetof(msptiActivityCommunication, dataType)},
    {"count", FieldType::INTEGER, offsetof(msptiActivityCommunication, count)},
    {"deviceId", FieldType::INTEGER, offsetof(msptiActivityCommunication, ds.deviceId)},
    {"streamId", FieldType::INTEGER, offsetof(msptiActivityCommunication, ds.streamId)},
    {"start", FieldType::INTEGER, offsetof(msptiActivityCommunication, start)},
    {"end", FieldType::TEXT, offsetof(msptiActivityCommunication, end)},
    {"algType", FieldType::TEXT, offsetof(msptiActivityCommunication, algType)},
    {"name", FieldType::TEXT, offsetof(msptiActivityCommunication, name)},
    {"commName", FieldType::TEXT, offsetof(msptiActivityCommunication, commName)},
    {"correlationId", FieldType::TEXT, offsetof(msptiActivityCommunication, correlationId)}
});
