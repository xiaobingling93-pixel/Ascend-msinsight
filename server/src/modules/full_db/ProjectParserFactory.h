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


namespace Dic::Module {
using namespace Dic::Module::Global;
class ProjectParserBase {
public:
    ProjectParserBase() = default;
    virtual ~ProjectParserBase() = default;
    virtual void Parser(const std::vector<Global::ProjectExplorerInfo> &projectInfos, ImportActionRequest &request)
    {
        // 需要返回应答
        auto responsePtr = std::make_unique<ImportActionResponse>();
        ImportActionResponse& response  = *responsePtr;
        responsePtr->body.isCluster = false;
        ModuleRequestHandler::SetBaseResponse(request, response);
        response.body.subParseFileInfo = {};
        response.command = Protocol::REQ_RES_IMPORT_ACTION;
        response.moduleName  = MODULE_TIMELINE;
        response.body.reset = false;
        ModuleRequestHandler::SetResponseResult(response, true);
        SendImportActionRes(std::move(responsePtr));
    };
    virtual void ParserBaseline(const Global::ProjectExplorerInfo &projectInfo,
        Global::BaselineInfo &baselineInfo)
    {
        return;
    }
    virtual ProjectTypeEnum GetProjectType(const std::string &dataPath)
    {
        return  ProjectTypeEnum::OTHER;
    };
    virtual std::vector<std::string> GetParseFileByImportFile(const std::string &importFile, std::string &error)
    {
        std::vector<std::string> res = { importFile };
        // other默认弹窗,存在以下场景:由于存在非法路径, DB JSON等格式会判断异常走到other下
        error = "No parsable files found, Possible reasons:; 1.File not exist; "
                "2.The nesting depth of the imported sub-file exceeds 5; 3.The sub-file path length exceeds " +
                std::to_string(FileUtil::GetFilePathLengthLimit());
        return res;
    };
    static void ParseEndCallBack(const std::string &rankId,
                                 const std::string &fileId,
                                 bool result,
                                 const std::string &message);
    static void ParseProgressCallBack(const std::string &fileId, uint64_t parsedSize, uint64_t totalSize, int progress);
    static void ParsePostProcess(const std::vector<std::shared_ptr<ParseFileInfo>> &clusterInfos);
    static bool ParseHeatMapToCluster(const std::vector<std::shared_ptr<ParseFileInfo>> &clusterInfos);
    static void SendAllParseSuccess();
    static bool CheckIsOpenClusterTag(ProjectActionEnum action, ProjectTypeEnum curType,
                                      const std::string &projectName);
    static bool IsParsedFile(const std::string& file);
    static void BuildProjectExploreInfo(ProjectExplorerInfo& projectInfo, const std::vector<std::string>& parsedFiles);

    /**
     * @brief 用于构造项目工程目录时获取上层目录的遍历结果
     */
    static std::vector<std::string> GetParentFileList(const std::string& prefix, const std::string& filePath);

    static std::tuple<std::string, std::string> GetClusterInfo(const std::vector<std::string>& folders);

    static std::string GetSubId(const std::string &str, [[maybe_unused]] ParseFileType type)
    {
        if (str.empty()) {
            return "";
        }
        if (FileUtil::IsFolder(str)) {
            return str;
        }
        return FileUtil::GetFileName(str);
    }

    static std::vector<std::string> SearchDeviceInfo(const std::string &searchPath);
    static std::vector<std::string> ParseDeviceInfo(ProjectExplorerInfo& info, const std::string& searchPath);
    static std::set<std::string> ParseDeviceIdSetFromCsv(const std::string& filePath);
    static std::set<std::string> ParseDeviceIdSetFromDb(const std::string& dbPath);
    static void AddRankDeviceParseFileInfo(ProjectExplorerInfo& info, std::shared_ptr<ParseFileInfo> rankInfo);
    std::string GetRankIdFromPath(const std::string &filePath, const std::string &importPath);
    static std::string GetDbPath(const std::string &filePath, const int index);

    static void SendUnitFinishNotify(const std::string &fileId, bool res, const std::string &unitName,
                                     const std::string &error = "");

protected:
    std::string curScene;
    std::map<std::string, std::vector<std::string>> dataPathToDbMap;
    std::unique_ptr<IFileReader> fileReader = nullptr;

    static void ParseClusterEndProcess(std::string result, bool isShowCluster, const std::string &clusterId);
    static void SearchMetaData(const std::string &rankId, const std::string &fileId,
                               std::vector<std::unique_ptr<UnitTrack>> &metaData);
    static void ProcessMetadata(std::vector<std::unique_ptr<UnitTrack>> &metaData);
    static void SendParseSuccessEvent(const std::string &rankId, const std::string &fileId);
    static void SendParseFailEvent(const std::string &rankId,
                                   const std::string &fileId,
                                   const std::string &message);
    static bool IsNeedReset(const ImportActionRequest &request);

    void SetBaseActionOfResponse(ImportActionResponse &response,
                                 const std::string &rankId,
                                 const std::string &fileId,
                                 const std::string &cardPath,
                                 std::vector<std::string> dataPath);
    static void SaveDbPath(const std::string &curProjectName,
        std::map<std::string, std::vector<std::string>> &dataPathToDbMap);

    static bool IsMindFormsRankData(const std::vector<std::string>& parentFolders);
    void SendImportActionRes(std::unique_ptr<ImportActionResponse> responsePtr);
};

class ParserFactory {
public:
    static std::shared_ptr<ProjectParserBase> GetProjectParser(ParserType allocType);
    static std::pair<std::string, ParserType> GetImportType(const std::string &path);
    static void Reset();

private:
    static std::mutex mutex;
};
} // end of namespace Module
// end of namespace Dic


#endif // PROFILER_SERVER_PARSERFACTORY_H
