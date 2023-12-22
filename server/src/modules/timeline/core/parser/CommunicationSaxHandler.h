/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 */

#ifndef PROFILER_SERVER_COMMUNICATIONSAXHANDLER_H
#define PROFILER_SERVER_COMMUNICATIONSAXHANDLER_H

#include "json.hpp"
#include "ClusterDatabase.h"
#include "GlobalDefs.h"
#include "DataBaseManager.h"

namespace Dic {
namespace Module {
namespace Timeline {
class CommunicationSaxHandler : public json_t::json_sax_t {
public:
    CommunicationSaxHandler();

    ~CommunicationSaxHandler() override;

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

    CommunicationBandWidth MapToBandwidth(const json_t &json);
    CommunicationTimeInfo MapToTimeInfo(const json_t &json);

private:
    float tempFloat = 0;
    int tempInt = 0;
    json_t currentObject;
    json_t sizeDistribution;
    std::string currentKey;
    std::string rankId;
    std::string stepId;
    std::string stageId;
    std::string tempOpName;
    std::string transportType;
    std::string tableFlag;
    std::string selectedPath;
    int currentDepth = 0;
    int stageIdDepth = 1;
    int stepIdDepth = 2;
    int tempOpNameDepth = 3;
    int rankIdDepth = 4;
    int stepSubLen = 4;
    int tableFlagDepth = 5;
    int infoDepth = 6;
    int infoDepthSeven = 7;
    int sizeDistributionDepth = 8;
    std::string exception;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif

// PROFILER_SERVER_COMMUNICATIONSAXHANDLER_H