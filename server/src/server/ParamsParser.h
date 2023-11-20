/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_PARAMS_PARSER_H
#define DATA_INSIGHT_CORE_PARAMS_PARSER_H

#include <string>
#include <vector>

namespace Dic {
namespace Server {
using namespace std;

struct ParamsOption {
    int wsPort = -1;
    int logSize = 32 * 1024 * 1024;
    string host = "0.0.0.0";
    string sid;
    string logLevel = "INFO";
    string logPath = "./";
    int scanPort = -1;
};

class ParamsParser {
public:
    static ParamsParser &Instance();
    const ParamsOption &GetOption() const;
    const std::string &GetError() const;
    const bool Parse(const vector<string> &args);

private:
    ParamsParser() = default;
    ~ParamsParser() = default;

    int TryGetPort(const std::string &portStr) const;
    bool ParseField(const std::string &data);
    bool ParseWsPort(const std::string &wsPortStr);
    bool ParseLogPath(const std::string &logPath);
    bool ParseLogSize(const std::string &logSize);
    bool ParseLogLevel(const std::string &logLevel);
    bool ParseSid(const std::string &sid);
    void ParseScan(const std::string &scan);

    const string symbolWsPort = "--wsPort=";
    const string symbolLogPath = "--logPath=";
    const string symbolLogSize = "--logSize=";
    const string symbolLogLevel = "--logLevel=";
    const string symbolSid = "--sid=";
    const string symbolScan = "--scan=";
    const int minPortNum = 9000;
    const int maxPortNum = 9100;

    ParamsOption option;
    std::string error;
};
} // end of namespace Server
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_PARAMS_PARSER_H
