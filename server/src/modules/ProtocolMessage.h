/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol declaration
 */

#ifndef DIC_PROTOCOL_BASE_H
#define DIC_PROTOCOL_BASE_H

#include "ProtocolUtil.h"
#include "StringUtil.h"
#include "NumDefs.h"

namespace Dic {
const int REQUEST_PARAMS_ERROR = 4003;
namespace Protocol {
inline bool CheckPageValid(int64_t pageSize, int64_t currentPage, std::string &errorMsg)
{
    if (pageSize <= MIN_PAGESIZE || pageSize > MAX_PAGESIZE) {
        errorMsg = "pagesize: " + std::to_string(pageSize) + " is invalid";
        return false;
    }
    if (currentPage <= MIN_CURRENT_PAGE || currentPage > MAX_CURRENT_PAGE) {
        errorMsg = "current page: " + std::to_string(currentPage) + " is invalid";
        return false;
    }
    return true;
}

inline bool CheckUnsignPageValid(uint64_t pageSize, uint64_t currentPage, std::string &errorMsg)
{
    if (pageSize > INT64_MAX || currentPage > INT64_MAX) {
        errorMsg = "page size or current page is too big";
        return false;
    }
    return CheckPageValid(pageSize, currentPage, errorMsg);
}

// 和CheckStrParamValid不同，该函数不做参数长度限制
inline bool CheckStrParamValidWithoutLenLimit(const std::string &param,
                                              std::string &errorMsg)
{
    if (param.empty()) {
        errorMsg = "Parameter is empty.";
        return false;
    }
    if (!StringUtil::ValidateStringParam(param)) {
        errorMsg = "Parameter contains illegal character, such as '|', ';', '&', '$', etc.";
        return false;
    }
    return true;
}

// arguments will be placed into specified request
inline bool CheckStrParamValid(const std::string &param,
    std::string &errorMsg)
{
    constexpr unsigned maxStrLength = 500;
    if (param.size() > maxStrLength) {
        errorMsg = "Parameter length exceeds the upper limit " + std::to_string(maxStrLength) + ".";
        return false;
    }
    if (param.empty()) {
        errorMsg = "Parameter is empty.";
        return false;
    }
    if (!StringUtil::ValidateStringParam(param)) {
        errorMsg = "Parameter contains illegal character, such as '|', ';', '&', '$', etc.";
        return false;
    }
    return true;
}

// 和CheckStrParamValid不同，该函数认为空字符串是合法的
inline bool CheckStrParamValidEmptyAllowed(const std::string &param,
    std::string &errorMsg)
{
    constexpr unsigned maxStrLength = 500;
    if (param.size() > maxStrLength) {
        errorMsg = "Parameter length exceeds the upper limit " + std::to_string(maxStrLength) + ".";
        return false;
    }
    if (!StringUtil::ValidateStringParam(param)) {
        errorMsg = "Parameter contains illegal character, such as '|', ';', '&', '$', etc.";
        return false;
    }
    return true;
}
} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_PROTOCOL_BASE_H
