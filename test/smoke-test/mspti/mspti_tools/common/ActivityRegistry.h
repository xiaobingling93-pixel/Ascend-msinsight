#pragma once
#include <string>
#include <vector>
#include <cstddef>
#include <typeinfo>
#include <stdexcept>
#include "FieldType.h"

struct FieldDef {
    std::string name;
    FieldType type;
    size_t offset;
};

struct TableInfo {
    std::string tableName;
    std::vector<FieldDef> fields;
    size_t typeHash;
};

std::vector<TableInfo>& getRegistry();

template<typename T>
struct ActivityTraits; // 每个类型的特化会提供字段映射和表名

#define REGISTER_ACTIVITY_TYPE(TYPE, TABLE, FIELDS) \
template<> struct ActivityTraits<TYPE> { \
    static inline const std::string tableName = TABLE; \
    static inline const std::vector<FieldDef> fields = FIELDS; \
}

template<typename T>
struct ActivityTraits {
    static TableInfo& get() {
        size_t h = typeid(T).hash_code();
        for (auto& t : getRegistry()) {
            if (t.typeHash == h) return t;
        }
        throw std::runtime_error("ActivityTraits not registered for this type " + std::string(typeid(T).name()));
    }
};

#define REGISTER_ACTIVITY_TYPE(TYPE, TABLE, FIELDS) \
    static bool _reg_##TYPE = [](){ \
        getRegistry().push_back({TABLE, FIELDS, typeid(TYPE).hash_code()}); \
        return true; \
    }(); \
    template<> struct ActivityTraits<TYPE> { \
        static TableInfo& get() { \
            size_t h = typeid(TYPE).hash_code(); \
            for (auto& t : getRegistry()) { \
                if (t.typeHash == h) return t; \
            } \
            throw std::runtime_error("ActivityTraits not registered for this type"); \
        } \
    };
