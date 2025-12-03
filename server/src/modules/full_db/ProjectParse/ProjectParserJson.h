/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_PARSERJSON_H
#define PROFILER_SERVER_PARSERJSON_H

#include "ProjectParserFactory.h"
#include "TimelineRequestHandler.h"
#include "ClusterFileParser.h"
#include "FileParser.h"

namespace Dic {
namespace Module {
class ProjectParserJson : public ProjectParserBase {
public:
    ProjectParserJson();

    ~ProjectParserJson() override = default;

    void Parser(const std::vector<Global::ProjectExplorerInfo> &projectInfos,
                ImportActionRequest &request,
                ImportActionResponse &response) final;

    void ParserBaseline(const Global::ProjectExplorerInfo &projectInfo,
                        Global::BaselineInfo &baselineInfo) final;

    ProjectTypeEnum GetProjectType(const std::string &dataPath) final;

    std::vector<std::string> GetParseFileByImportFile(const std::string &importFile, std::string &error) final;

    static bool ExistJsonFormatFile(const std::string &file);

    static void BuildProjectExploreInfo(ProjectExplorerInfo &info, const std::vector<std::string> &parsedFiles);

    static void BuildProjectFromParseFile(ProjectExplorerInfo &info, const std::string &parseFile);

    static std::string GetSubId(const std::string &filePath, ParseFileType type);

    static std::string GetDeviceId(const std::string &parseFolder, const std::string &rankId);

    static std::string GetDeviceIdFromMemory(const std::string& parseFolder);

    static std::string GetDeviceIdFromKernel(const std::string& parseFolder);

    static std::string GetDeviceIdFromCSVFile(const std::string& filePath);

    static std::string GetDeviceIdFromPath(const std::string& parseFolder);

protected:
    bool CheckParseFileInfoSize(const std::shared_ptr<Global::ParseFileInfo> &parseFileInfo,
                                std::vector<std::string> &jsonFiles) const;

    static std::tuple<bool, bool, bool> CheckHasTraceJsonMemoryDataOperatorData(
            const std::vector<Global::ProjectExplorerInfo> &projectInfos);

    static std::string GetFileIdWithDb(const std::string &filePath);

private:
    std::vector<std::string> FindAllTraceFile(const std::string &path, std::string &error);

    static std::vector<std::string> FindTraceFile(const std::string &path, std::string &error, std::string &curScene);

    static void FindTraceFiles(const std::string &path, int depth, std::string &error,
                               std::vector<std::string> &traceFiles,
                               std::string &curScene);

    static void FindAscendFolder(const std::string &path, std::vector<std::string> &traceFiles);

    static bool IsJsonValid(const std::string &fileName);

    static void ClusterProcess(std::shared_ptr<ParseFileInfo> clusterInfo,
                               ProjectTypeEnum projectType,
                               bool isShowCluster,
                               std::map<std::string, std::vector<std::string>> &dataPathToDbMap,
                               const std::string &projectName);

    static void ClusterProcessAsyncStep(Timeline::ClusterFileParser clusterFileParser);

    void SetParseCallBack(FileParser &fileParser);

    bool isSimulation(std::string filePath);

    std::map<std::string, RankEntry>
    GetRankEntryMap(const std::vector<Global::ProjectExplorerInfo> &projectInfos, bool isBaseline);

    std::string AddSuffixToDuplicatedRankId(const std::map<std::string, RankEntry> &rankToTraceMap,
                                            const std::string &rankId);

    std::vector<std::string> GetJsonFileUnderFolder(const std::string &path);

    void ParserTraceData(const std::map<std::string, RankEntry> &rankListMap,
                         const std::vector<Global::ProjectExplorerInfo> &projectInfos, bool isShowCluster);

    static void FillBaseResponseInfo(const ImportActionRequest &request, ImportActionResponse &response,
                                     const std::vector<Global::ProjectExplorerInfo> &projectInfos);

    static void ParserClusterBaseline(const Global::ProjectExplorerInfo &projectInfo, BaselineInfo &baselineInfo);

    void ParserSingleCardBaseline(const Global::ProjectExplorerInfo &projectInfos,
                                  Global::BaselineInfo &baselineInfo);

    static void ParserMetaData(const std::vector<Global::ProjectExplorerInfo> &projectInfos);

    static void UpdateRankIdToDevice(std::map<std::string, RankEntry> &rankEntry);

    void SetBaseAction(const std::map<std::string, RankEntry> &rankListMap, ImportActionResponse &response);

    void ParseBaselineTraceFile(const std::vector<std::string> &jsonFiles, const std::string &rankId,
                                const std::string &fileId, const std::string &filePath);
};
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_PARSERJSON_H
