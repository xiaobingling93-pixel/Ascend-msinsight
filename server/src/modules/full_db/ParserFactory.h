/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_PARSERFACTORY_H
#define PROFILER_SERVER_PARSERFACTORY_H


#include <iostream>
#include <memory>
#include <map>
#include "TimelineRequestHandler.h"
#include "GlobalDefs.h"
#include "IFileReader.h"
#include "SystemMemoryDatabaseDef.h"
#include "TimelineProtocolEvent.h"

namespace Dic {
namespace Module {
class ParserAlloc {
public:
    ParserAlloc() = default;
    virtual ~ParserAlloc() = default;
    virtual void Parser(const std::vector<Global::ProjectExplorerInfo> &projectInfos, ImportActionRequest &request) {};
    virtual void ParserBaseline(const std::vector<Global::ProjectExplorerInfo> &projectInfos,
        Global::BaselineInfo &baselineInfo)
    {
        return;
    }
    virtual ProjectTypeEnum GetProjectType(const std::vector<std::string> &dataPath)
    {
        return  ProjectTypeEnum::OTHER;
    };
    virtual std::vector<std::string> GetParseFileByImportFile(const std::string &importFile,
        ProjectTypeEnum projectTypeEnum, std::string &error)
    {
        std::vector<std::string> res = { importFile };
        return res;
    };
    static void ParseEndCallBack(const std::string &fileId, bool result, const std::string &message);
    static void ParseProgressCallBack(const std::string &fileId, uint64_t parsedSize, uint64_t totalSize, int progress);
    static void SendAllParseSuccess();
    static bool CheckIsOpenClusterTag(ProjectActionEnum action, ProjectTypeEnum curType,
                                      const std::string &projectName);

protected:
    std::string curScene;
    std::map<std::string, std::vector<std::string>> dataPathToDbMap;
    std::unique_ptr<IFileReader> fileReader = nullptr;

    static void ParseClusterEndProcess(std::string result, bool isShowCluster);
    static void SearchMetaData(const std::string &fileId, std::vector<std::unique_ptr<UnitTrack>> &metaData);
    std::string GetFileId(const std::string &filePath, const std::string &importPath);
    static std::string GetDbPath(const std::string &filePath, const int index);
    static void SendParseSuccessEvent(const std::string &fileId);
    static void SendParseFailEvent(const std::string &fileId, const std::string &message);
    static bool IsNeedReset(const ImportActionRequest &request);

    void SetBaseActionOfResponse(ImportActionResponse &response, const std::string &rankId, const std::string &cardPath,
        std::vector<std::string> dataPath);
    static void SaveDbPath(const std::string &curProjectName,
        std::map<std::string, std::vector<std::string>> &dataPathToDbMap);
};

class ParserFactory {
public:
    static std::shared_ptr<ParserAlloc> ParserImport(ParserType allocType);
    static std::pair<std::string, ParserType> GetImportType(const std::vector<std::string> &pathList);
    static void Reset();

private:
    static std::mutex mutex;
};
} // end of namespace Module
} // end of namespace Dic


#endif // PROFILER_SERVER_PARSERFACTORY_H
