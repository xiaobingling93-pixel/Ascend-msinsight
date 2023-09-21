/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 */

#ifndef PROFILER_SERVER_COMMUNICATIONMATRIXHANDLER_H
#define PROFILER_SERVER_COMMUNICATIONMATRIXHANDLER_H

#include "json.hpp"
#include "ClusterDatabase.h"
#include "GlobalDefs.h"
#include "DataBaseManager.h"

namespace Dic {
namespace Module {
namespace Timeline {
class CommunicationMatrixHandler : public json_t::json_sax_t {
public:
    CommunicationMatrixHandler();

    ~CommunicationMatrixHandler() override;

    bool start_object(std::size_t elements) override;
    bool boolean(bool val) override;
    bool number_integer(number_integer_t val) override;
    bool number_unsigned(number_unsigned_t val) override;
    bool number_float(number_float_t val, const string_t &s) override;
    bool string(string_t &val) override;
    bool key(string_t &val) override;
    bool end_object() override;
    bool null() override;
    bool binary(binary_t &val) override;
    bool start_array(std::size_t elements) override;
    bool end_array() override;
    bool parse_error(std::size_t position, const std::string &last_token,
                     const nlohmann::detail::exception &ex) override;

    CommunicationMatrixInfo MapToMatrixInfo(const json_t &json);

private:

    std::string exception;
    int currentDepth = 0;
    json_t currentObject;
    std::string currentKey;
    std::string groupId;
    std::string iterationId;
    std::string tempOpName;
    std::string tempRank;
    int groupDepth = 1;
    int stepDepth = 2;
    int opNameDepth = 3;
    int ranksDepth = 4;
    int stepSubLen = 4;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif

// PROFILER_SERVER_COMMUNICATIONMATRIXHANDLER_H