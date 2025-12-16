#pragma once
#include <string>

enum class FieldType { INTEGER_32, INTEGER, REAL, TEXT };

inline std::string sqlType(FieldType t) {
    switch (t) {
        case FieldType::INTEGER_32: return "INTEGER";
        case FieldType::INTEGER: return "INTEGER";
        case FieldType::REAL: return "REAL";
        case FieldType::TEXT: return "TEXT";
    }
    return "TEXT";
}
