//
// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//

#ifndef PROFILER_SERVER_PARSERIPYNB_H
#define PROFILER_SERVER_PARSERIPYNB_H

#include "ParserFactory.h"

namespace Dic {
namespace Module {
class ParserIpynb : public ParserAlloc {
public:
    ParserIpynb();
    virtual ~ParserIpynb();

    void Parser(const std::vector<Global::ProjectExplorerInfo> &projectInfos, ImportActionRequest &request) final;
    ProjectTypeEnum GetProjectType(const std::vector<std::string> &dataPath) final;
private:
    static void IpynbImportResponse(ImportActionRequest &request);
    static void JupyterProcess(const std::string &token, const std::string &file);
    static void SendJupyterInfo(const std::string &token, std::string url);
};
}
}
#endif // PROFILER_SERVER_PARSERIPYNB_H
