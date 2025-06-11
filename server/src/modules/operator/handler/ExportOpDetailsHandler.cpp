/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#ifdef __APPLE__
#include <filesystem>
#include <sys/stat.h>
#elif __linux__
#include <sys/stat.h>
#endif
#include "pch.h"
#include "BaselineManager.h"
#include "DataBaseManager.h"
#include "OperatorProtocolRequest.h"
#include "OperatorProtocolResponse.h"
#include "OperatorGroupConverter.h"
#include "WsSessionManager.h"
#include "ProjectExplorerManager.h"
#include "QueryOpStatisticInfoHandler.h"
#include "QueryOpDetailInfoHandler.h"
#include "ExportOpDetailsHandler.h"
#include "NumberSafeUtil.h"

namespace {
    using namespace Dic;
    using namespace Dic::Server;

    struct HeaderAndColConcatStatisticInfo {
        std::vector<std::string> header;
        std::function<std::string(const OperatorStatisticInfoRes&)> colConcatFunc;
    };
    struct HeaderAndColConcatDetailInfo {
        std::vector<std::string> header;
        std::function<std::string(const OperatorDetailInfoRes&)> colConcatFunc;
    };

    std::string CheckColumn(const std::string &column)
    {
        if (NumberUtil::IsDouble(column)) {
            return column;
        }

        std::regex re(R"(^[\=\+\-\@].*)");
        if (std::regex_match(column, re)) {
            return "\\" + column;
        } else {
            return column;
        }
    }
    // LCOV_EXCL_BR_START
    std::vector<std::string> operatorTypeHeader = { "opType", "accCore", "count", "totalTime", "avgTime", "maxTime",
                                                    "minTime" };
    std::string OperatorTypeColConcatFun(const OperatorStatisticInfoRes& data)
    {
        std::string csvStr = CheckColumn(data.opType) + "," + CheckColumn(data.accCore) + ","
                + CheckColumn(data.count) + "," + CheckColumn(data.totalTime) + ","
                + CheckColumn(data.avgTime) + "," + CheckColumn(data.maxTime) + ","
                + CheckColumn(data.minTime) + "\n";
        return csvStr;
    }

    std::vector<std::string> inputShapeHeader = { "opName", "inputShape", "accCore", "count", "totalTime", "avgTime",
                                                  "maxTime", "minTime" };
    std::string InputShapeColConcatFun(const OperatorStatisticInfoRes& data)
    {
        std::string csvStr = CheckColumn(data.opName) + "," + CheckColumn(data.inputShape) + ","
                + CheckColumn(data.accCore) + "," + CheckColumn(data.count) + ","
                + CheckColumn(data.totalTime) + "," + CheckColumn(data.avgTime) + ","
                + CheckColumn(data.maxTime) + "," + CheckColumn(data.minTime) + "\n";
        return csvStr;
    }

    std::vector<std::string> communicationOpTypeHeader = { "opType", "count", "totalTime", "avgTime", "maxTime",
                                                           "minTime" };
    std::string CommunicationOpTypeColConcatFun(const OperatorStatisticInfoRes& data)
    {
        std::string csvStr = CheckColumn(data.opType) + "," + CheckColumn(data.count) + ","
                + CheckColumn(data.totalTime) + "," + CheckColumn(data.avgTime) + ","
                + CheckColumn(data.maxTime) + "," + CheckColumn(data.minTime) + "\n";
        return csvStr;
    }

    std::vector<std::string> operatorHeader = { "name", "type", "accCore", "startTime", "duration", "waitTime",
                                                "blockDim", "inputShape", "inputType", "inputFormat", "outputShape",
                                                "outputType", "outputFormat" };
    std::string OperatorColConcatFun(const OperatorDetailInfoRes& data)
    {
        std::string csvStr = CheckColumn(data.name) + "," + CheckColumn(data.type) + ","
                + CheckColumn(data.accCore) + "," + CheckColumn(data.startTime) + ","
                + CheckColumn(data.duration) + ","+ CheckColumn(data.waitTime)+ ","
                + CheckColumn(data.blockDim) + "," + CheckColumn(data.inputShape) + ","
                + CheckColumn(data.inputType) + "," + CheckColumn(data.inputFormat) + ","
                + CheckColumn(data.outputShape) + "," + CheckColumn(data.outputType) + ","
                + CheckColumn(data.outputFormat);
        for (const auto &value : data.pmuDatas) {
            csvStr += "," + CheckColumn(value.second);
        }
        csvStr += "\n";
        return csvStr;
    }

    std::vector<std::string> CommunicationOpHeader = { "name", "type", "startTime", "duration", "waitTime" };
    std::string CommunicationOpColConcatFun(const OperatorDetailInfoRes& data)
    {
        std::string csvStr = CheckColumn(data.name) + "," + CheckColumn(data.type) + ","
                + CheckColumn(data.startTime)+ "," + CheckColumn(data.duration) + ","
                + CheckColumn(data.waitTime) + "\n";
        return csvStr;
    }

    std::string concatHeader(const std::vector<std::string> header,
                             const std::set<std::string> pmuHeaders = {})
    {
        std::string headerStr = StringUtil::join(header, ",");
        for (const auto &value : pmuHeaders) {
            headerStr += "," + value;
        }
        headerStr += "\n";
        return headerStr;
    }
    // LCOV_EXCL_BR_STOP

    std::map<std::string, HeaderAndColConcatStatisticInfo> CsvHandleStatisticInfoMap = {
        { "Operator Type",  { header: operatorTypeHeader, colConcatFunc: OperatorTypeColConcatFun } },
        { "Input Shape",  { header: inputShapeHeader, colConcatFunc: InputShapeColConcatFun }},
        { "Communication Operator Type",  { header: communicationOpTypeHeader,
                                            colConcatFunc: CommunicationOpTypeColConcatFun }},
    };
    std::map<std::string, HeaderAndColConcatDetailInfo> CsvHandleDetailInfoMap = {
        { "Operator",  { header: operatorHeader, colConcatFunc: OperatorColConcatFun }},
        { "Communication Operator",  { header: CommunicationOpHeader, colConcatFunc: CommunicationOpColConcatFun }},
    };
};

namespace Dic::Module::Operator {
    using namespace Dic::Server;

    bool ExportOpDetailsHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
    {
        OperatorExportDetailsRequest &request = dynamic_cast<OperatorExportDetailsRequest &>(*requestPtr);
        std::unique_ptr<OperatorExportDetailsResponse> responsePtr = std::make_unique<OperatorExportDetailsResponse>();
        OperatorExportDetailsResponse &response = *responsePtr;
        SetBaseResponse(request, response);

        std::string errMsg;
        if (!request.params.CommonCheck(errMsg) || !request.params.StatisticGroupCheck(errMsg)) {
            ServerLog::Error(errMsg);
            SendResponse(std::move(responsePtr), false);
            return false;
        }

        bool rst = request.params.isCompare ?
                HandleCompareDataRequest(request, dynamic_cast<OperatorExportDetailsResponse &>(*responsePtr)) :
                HandleDataRequest(request, dynamic_cast<OperatorExportDetailsResponse &>(*responsePtr));
        SendResponse(std::move(responsePtr), rst);
        return true;
    }

    bool ExportOpDetailsHandler::HandleCompareDataRequest(OperatorExportDetailsRequest &request,
                                                          OperatorExportDetailsResponse &response)
    {
        if (request.params.IsStatisticGroup()) {
            HandleStatisticCompareDataRequest(request, response);
            return true;
        }
        if (request.params.IsNotStatisticGroup()) {
            HandleNotStatisticCompareDataRequest(request, response);
            return true;
        }
        return false;
    }

    bool ExportOpDetailsHandler::HandleStatisticCompareDataRequest(OperatorExportDetailsRequest &request,
                                                                   OperatorExportDetailsResponse &response)
    {
        QueryOpStatisticInfoHandler handler;
        OperatorStatisticInfoRequest operatorStatisticCompareReq;
        operatorStatisticCompareReq.params = {
                isCompare: request.params.isCompare,
                rankId: request.params.rankId,
                deviceId: request.params.deviceId,
                group: request.params.group,
                topK: request.params.topK,
                current: 1,
                pageSize: INT64_MAX,
        };
        OperatorStatisticInfoResponse operatorStatisticCompareResponse;
        if (!handler.HandleCompareDataRequest(operatorStatisticCompareReq, operatorStatisticCompareResponse)) {
            ServerLog::Error("[Operator]Failed to query current Statistic Info in export op detail.");
            return false;
        }
        auto it = CsvHandleStatisticInfoMap.find(request.params.group);
        if (it == CsvHandleStatisticInfoMap.end()) {
            ServerLog::Error("[Operator]Failed to get CsvHandle in export op detail.");
            return false;
        }
        CreateCsvFile(request, response);
        std::string headerStr = concatHeader(it->second.header);
        AppendFileContent(headerStr);
        for (auto &dataItem : operatorStatisticCompareResponse.datas) {
            std::string csvStr = it->second.colConcatFunc(dataItem.diff);
            if (!AppendFileContent(csvStr)) {
                response.exceedingFileLimit = true;
                DestroyFile();
                return true;
            };
        }
        DestroyFile();
        return true;
    }

    bool ExportOpDetailsHandler::HandleNotStatisticCompareDataRequest(OperatorExportDetailsRequest &request,
                                                                      OperatorExportDetailsResponse &response)
    {
        QueryOpDetailInfoHandler handler;
        OperatorDetailInfoRequest operatorNotStatisticCompareReq;
        operatorNotStatisticCompareReq.params = {
                isCompare: request.params.isCompare,
                rankId: request.params.rankId,
                deviceId: request.params.deviceId,
                group: request.params.group,
                topK: request.params.topK,
                current: 1,
                pageSize: INT64_MAX,
        };
        OperatorDetailInfoResponse operatorNotStatisticCompareResponse;
        if (!handler.HandleCompareDataRequest(operatorNotStatisticCompareReq, operatorNotStatisticCompareResponse)) {
            ServerLog::Error("[Operator]Failed to query current Statistic Info in export op detail.");
            return false;
        }
        // LCOV_EXCL_BR_START
        auto it = CsvHandleDetailInfoMap.find(request.params.group);
        if (it == CsvHandleDetailInfoMap.end()) {
            ServerLog::Error("[Operator]Failed to get CsvHandle in export op detail.");
            return false;
        }
        CreateCsvFile(request, response);
        std::string headerStr = request.params.group == "Operator"
                ? concatHeader(it->second.header, operatorNotStatisticCompareResponse.pmuHeaders)
                : concatHeader(it->second.header);

        AppendFileContent(headerStr);
        for (auto &dataItem : operatorNotStatisticCompareResponse.datas) {
            std::string csvStr = it->second.colConcatFunc(dataItem.diff);
            if (!AppendFileContent(csvStr)) {
                response.exceedingFileLimit = true;
                DestroyFile();
                return true;
            };
        }
        DestroyFile();
        // LCOV_EXCL_BR_STOP
        return true;
    }

    bool ExportOpDetailsHandler::HandleDataRequest(OperatorExportDetailsRequest &request,
                                                   OperatorExportDetailsResponse &response)
    {
        if (request.params.IsStatisticGroup()) {
            HandleStatisticDataRequest(request, response);
            return true;
        }
        if (request.params.IsNotStatisticGroup()) {
            HandleNotStatisticDataRequest(request, response);
            return true;
        }
        return false;
    }

    bool ExportOpDetailsHandler::HandleStatisticDataRequest(OperatorExportDetailsRequest &request,
                                                            OperatorExportDetailsResponse &response)
    {
        std::string rankId = Summary::VirtualSummaryDataBase::GetFileIdFromCombinationId(request.params.rankId);
        auto database = Timeline::DataBaseManager::Instance().GetSummaryDatabaseByRankId(rankId);

        OperatorStatisticReqParams statisticReqParams = {
            isCompare: request.params.isCompare,
            rankId: request.params.rankId,
            deviceId: request.params.deviceId,
            group: request.params.group,
            topK: request.params.topK,
            current: 0,
            pageSize: 1000,
        };
        std::unique_ptr<OperatorStatisticInfoResponse> responsePtr = std::make_unique<OperatorStatisticInfoResponse>();
        OperatorStatisticInfoResponse &statisticReqResponse = *responsePtr;

        // LCOV_EXCL_BR_START
        auto it = CsvHandleStatisticInfoMap.find(request.params.group);
        if (it == CsvHandleStatisticInfoMap.end()) {
            ServerLog::Error("[Operator]Failed to get CsvHandle in export op detail.");
            return false;
        }
        CreateCsvFile(request, response);
        std::string headerStr = concatHeader(it->second.header);
        AppendFileContent(headerStr);
        do {
            statisticReqParams.current++;
            if (!database || !database->QueryOperatorStatisticInfo(statisticReqParams, statisticReqResponse)) {
                ServerLog::Error("[Operator]Failed to query Statistic Info in export op detail.");
                DestroyFile();
                return false;
            }
            for (auto &dataItem : statisticReqResponse.datas) {
                std::string csvStr = it->second.colConcatFunc(dataItem.compare);
                if (!AppendFileContent(csvStr)) {
                    response.exceedingFileLimit = true;
                    DestroyFile();
                    return true;
                };
            }
        } while (NumberSafe::Muls(statisticReqParams.current, statisticReqParams.pageSize) < statisticReqResponse.total
                 && NumberSafe::Muls(statisticReqParams.current, statisticReqParams.pageSize) > 0);
        DestroyFile();
        // LCOV_EXCL_BR_STOP
        return true;
    }

    bool ExportOpDetailsHandler::HandleNotStatisticDataRequest(OperatorExportDetailsRequest &request,
                                                               OperatorExportDetailsResponse &response)
    {
        std::string rankId = Summary::VirtualSummaryDataBase::GetFileIdFromCombinationId(request.params.rankId);
        auto database = Timeline::DataBaseManager::Instance().GetSummaryDatabaseByRankId(rankId);
        OperatorStatisticReqParams statisticReqParams = {
            isCompare: request.params.isCompare,
            rankId: request.params.rankId,
            deviceId: request.params.deviceId,
            group: request.params.group,
            topK: request.params.topK,
            current: 0,
            pageSize: 1000,
        };
        std::unique_ptr<OperatorDetailInfoResponse> responsePtr = std::make_unique<OperatorDetailInfoResponse>();
        OperatorDetailInfoResponse &detailInfoReqResponse = *responsePtr;

        auto it = CsvHandleDetailInfoMap.find(request.params.group);
        if (it == CsvHandleDetailInfoMap.end()) {
            ServerLog::Error("[Operator]Failed to get CsvHandle in export op detail.");
            return false;
        }
        CreateCsvFile(request, response);
        do {
            statisticReqParams.current++;
            if (!database || !database->QueryOperatorDetailInfo(statisticReqParams, detailInfoReqResponse)) {
                ServerLog::Error("[Operator]Failed to query detail Info in export op detail");
                DestroyFile();
                return false;
            }
            // LCOV_EXCL_BR_START
            if (statisticReqParams.current == 1) {
                std::string headerStr = request.params.group == "Operator"
                        ? concatHeader(it->second.header, detailInfoReqResponse.pmuHeaders)
                        : concatHeader(it->second.header);
                AppendFileContent(headerStr);
            }
            for (auto &dataItem : detailInfoReqResponse.datas) {
                std::string csvStr = it->second.colConcatFunc(dataItem.compare);
                if (!AppendFileContent(csvStr)) {
                    response.exceedingFileLimit = true;
                    DestroyFile();
                    return true;
                };
            }
            // LCOV_EXCL_BR_STOP
        } while (statisticReqParams.current * statisticReqParams.pageSize < detailInfoReqResponse.total);
        DestroyFile();
        return true;
    }

    void ExportOpDetailsHandler::CreateCsvFile(OperatorExportDetailsRequest &request,
                                               OperatorExportDetailsResponse &response)
    {
        std::string rankId = Summary::VirtualSummaryDataBase::GetFileIdFromCombinationId(request.params.rankId);
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

        std::vector<Global::ProjectExplorerInfo> projectInfo = Global::ProjectExplorerManager::Instance()
                .QueryProjectExplorer(request.projectName, {});
#ifdef _WIN32
const std::string_view MSVP_SLASH = "\\";
#else
const std::string_view MSVP_SLASH = "/";
#endif
        std::string filePath = projectInfo[0].fileName + std::string(MSVP_SLASH) + "operator_detail_group_by_"
                + request.params.group + "_" + rankId + "_" + std::to_string(timestamp) + ".csv";
        ofs.open(filePath, std::ios::out | std::ios::trunc);
        response.filePath = filePath;
        currentFileSize = 0;
#ifdef __APPLE__
        ofs.close(); // 关闭文件以设置权限
        std::filesystem::permissions(filePath, std::filesystem::perms::owner_read |
                std::filesystem::perms::owner_write | std::filesystem::perms::group_read);
        ofs.open(filePath, std::ofstream::out | std::ofstream::app); // 重新打开文件
#elif __linux__
        ofs.close();
        chmod(filePath.c_str(), S_IRUSR | S_IWUSR | S_IRGRP);
        ofs.open(filePath, std::ofstream::out | std::ofstream::app);
#endif
    }

    bool ExportOpDetailsHandler::AppendFileContent(const std::string &str)
    {
        if (ofs.is_open()) {
            ofs << str;
            ofs.flush();
            currentFileSize += str.length();
        }
        if (currentFileSize > maxFileSize) {
            ServerLog::Warn("[Operator]The file exceeds 1GB in export op detail");
            return false;
        }
        return true;
    }

    void ExportOpDetailsHandler::DestroyFile()
    {
        if (ofs.is_open()) {
            ofs.close();
        }
    }
}