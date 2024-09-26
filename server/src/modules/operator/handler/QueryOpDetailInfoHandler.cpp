/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include <algorithm>
#include "pch.h"
#include "BaselineManager.h"
#include "DataBaseManager.h"
#include "OperatorProtocolRequest.h"
#include "WsSessionManager.h"
#include "OperatorProtocol.h"
#include "QueryOpDetailInfoHandler.h"

namespace {
    using namespace Dic::Server;
    using DetailCmpRes = Protocol::OperatorDetailCmpInfoRes;
    using DetailCmpFun = std::function<bool(const DetailCmpRes&, const DetailCmpRes&)>;
    static std::unordered_map<std::string, DetailCmpFun> DetailDescCompareFunctions = {
        {"rank_id", [](const DetailCmpRes& a, const DetailCmpRes& b)
                    { return a.diff.rankId > b.diff.rankId; }},
        {"step_id", [](const DetailCmpRes& a, const DetailCmpRes& b)
                    { return a.diff.stepId > b.diff.stepId; }},
        {"name", [](const DetailCmpRes& a, const DetailCmpRes& b)
                 { return a.diff.name > b.diff.name; }},
        {"op_type", [](const DetailCmpRes& a, const DetailCmpRes& b)
                    { return a.diff.type > b.diff.type; }},
        {"accelerator_core", [](const DetailCmpRes& a, const DetailCmpRes& b)
                             { return a.diff.accCore > b.diff.accCore; }},
        {"start_time", [](const DetailCmpRes& a, const DetailCmpRes& b)
                       { return a.diff.startTime > b.diff.startTime; }},
        {"duration", [](const DetailCmpRes& a, const DetailCmpRes& b)
                     { return a.diff.duration > b.diff.duration; }},
        {"wait_time", [](const DetailCmpRes& a, const DetailCmpRes& b)
                      { return a.diff.waitTime > b.diff.waitTime; }},
        {"block_dim", [](const DetailCmpRes& a, const DetailCmpRes& b)
                      { return a.diff.blockDim > b.diff.blockDim; }},
        {"input_shapes", [](const DetailCmpRes& a, const DetailCmpRes& b)
                         { return a.diff.inputShape > b.diff.inputShape; }},
        {"input_data_types", [](const DetailCmpRes& a, const DetailCmpRes& b)
                             { return a.diff.inputType > b.diff.inputType; }},
        {"input_formats", [](const DetailCmpRes& a, const DetailCmpRes& b)
                          { return a.diff.inputFormat > b.diff.inputFormat; }},
        {"output_shapes", [](const DetailCmpRes& a, const DetailCmpRes& b)
                          { return a.diff.outputShape > b.diff.outputShape; }},
        {"output_data_types", [](const DetailCmpRes& a, const DetailCmpRes& b)
                              { return a.diff.outputType > b.diff.outputType; }},
        {"output_formats", [](const DetailCmpRes& a, const DetailCmpRes& b)
                           { return a.diff.outputFormat > b.diff.outputFormat; }},
    };
    static std::unordered_map<std::string, DetailCmpFun> DetailAsceCompareFunctions = {
        {"rank_id", [](const DetailCmpRes& a, const DetailCmpRes& b)
                    { return a.diff.rankId < b.diff.rankId; }},
        {"step_id", [](const DetailCmpRes& a, const DetailCmpRes& b)
                    { return a.diff.stepId < b.diff.stepId; }},
        {"name", [](const DetailCmpRes& a, const DetailCmpRes& b)
                 { return a.diff.name < b.diff.name; }},
        {"op_type", [](const DetailCmpRes& a, const DetailCmpRes& b)
                    { return a.diff.type < b.diff.type; }},
        {"accelerator_core", [](const DetailCmpRes& a, const DetailCmpRes& b)
                             { return a.diff.accCore < b.diff.accCore; }},
        {"start_time", [](const DetailCmpRes& a, const DetailCmpRes& b)
                       { return a.diff.startTime < b.diff.startTime; }},
        {"duration", [](const DetailCmpRes& a, const DetailCmpRes& b)
                     { return a.diff.duration < b.diff.duration; }},
        {"wait_time", [](const DetailCmpRes& a, const DetailCmpRes& b)
                      { return a.diff.waitTime < b.diff.waitTime; }},
        {"block_dim", [](const DetailCmpRes& a, const DetailCmpRes& b)
                      { return a.diff.blockDim < b.diff.blockDim; }},
        {"input_shapes", [](const DetailCmpRes& a, const DetailCmpRes& b)
                         { return a.diff.inputShape < b.diff.inputShape; }},
        {"input_data_types", [](const DetailCmpRes& a, const DetailCmpRes& b)
                             { return a.diff.inputType < b.diff.inputType; }},
        {"input_formats", [](const DetailCmpRes& a, const DetailCmpRes& b)
                          { return a.diff.inputFormat < b.diff.inputFormat; }},
        {"output_shapes", [](const DetailCmpRes& a, const DetailCmpRes& b)
                          { return a.diff.outputShape < b.diff.outputShape; }},
        {"output_data_types", [](const DetailCmpRes& a, const DetailCmpRes& b)
                              { return a.diff.outputType < b.diff.outputType; }},
        {"output_formats", [](const DetailCmpRes& a, const DetailCmpRes& b)
                           { return a.diff.outputFormat < b.diff.outputFormat; }},
    };
    bool DetailDescCmp(const DetailCmpRes& a, const DetailCmpRes& b, const std::string orderBy)
    {
        auto it = DetailDescCompareFunctions.find(orderBy);
        if (it != DetailDescCompareFunctions.end()) {
            return it->second(a, b);
        }
        return a.diff.duration > b.diff.duration;
    }
    bool DetailAsceCmp(const DetailCmpRes &a, const DetailCmpRes &b, const std::string orderBy)
    {
        auto it = DetailAsceCompareFunctions.find(orderBy);
        if (it != DetailAsceCompareFunctions.end()) {
            return it->second(a, b);
        }
        return a.diff.duration > b.diff.duration;
    }
    bool StaticCmp(const DetailCmpRes &a, const DetailCmpRes &b, const std::string order,
                   const std::string orderBy)
    {
        if (order == "ascend") {
            return DetailAsceCmp(a, b, orderBy);
        } else {
            return DetailDescCmp(a, b, orderBy);
        }
    }
};

namespace Dic::Module::Operator {
    using namespace Dic::Server;

    void QueryOpDetailInfoHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
    {
        OperatorDetailInfoRequest &request = dynamic_cast<OperatorDetailInfoRequest &>(*requestPtr);
        WsSession &session = *WsSessionManager::Instance().GetSession();
        std::unique_ptr<OperatorDetailInfoResponse> responsePtr = std::make_unique<OperatorDetailInfoResponse>();
        OperatorDetailInfoResponse &response = *responsePtr;
        bool rst = true;
        std::string errorMsg;
        if (request.params.CommonCheck(errorMsg)) {
            rst = request.params.isCompare ?
                HandleCompareDataRequest(request, dynamic_cast<OperatorDetailInfoResponse &>(*responsePtr)) :
                HandleDetailDataRequest(request, dynamic_cast<OperatorDetailInfoResponse &>(*responsePtr));
        }
        SetBaseResponse(request, response);
        SetResponseResult(response, rst);
        session.OnResponse(std::move(responsePtr));
    }

    bool QueryOpDetailInfoHandler::HandleCompareDataRequest(OperatorDetailInfoRequest &request,
                                                            OperatorDetailInfoResponse &response)
    {
        std::string rankId = Summary::VirtualSummaryDataBase::GetFileIdFromCombinationId(request.params.rankId);
        auto database = Timeline::DataBaseManager::Instance().GetSummaryDatabase(rankId);
        std::vector<Protocol::OperatorDetailInfoRes> cmpRes;
        if (!database->QueryAllOperatorDetailInfo(request.params, cmpRes, response.level)) {
            ServerLog::Error("[Operator]Failed to query current detail info by rankId.");
            return false;
        }
        if (request.params.topK == 0) {
            return true;
        }
        std::string baselineId = Global::BaselineManager::Instance().GetBaselineId();
        if (baselineId == "") {
            ServerLog::Error("[Operator]Failed to get baseline id.");
            return false;
        }
        auto databaseBaseline = DataBaseManager::Instance().GetSummaryDatabase(baselineId);
        std::vector<Protocol::OperatorDetailInfoRes> baselineRes;
        request.params.rankId = "";
        if (!databaseBaseline->QueryAllOperatorDetailInfo(request.params, baselineRes, response.level)) {
            ServerLog::Error("[Operator]Failed to query baseline detail Info by baselineId.");
            return false;
        }
        std::vector<Protocol::OperatorDetailCmpInfoRes> fullCmpData;
        fullCmpData = GetCmpDataVec(baselineRes, cmpRes);
        constexpr int64_t MAX_INT64 = std::numeric_limits<int64_t>::max();
        int64_t safeSize = (fullCmpData.size() > static_cast<size_t>(MAX_INT64)) ?
                            MAX_INT64 : static_cast<int64_t>(fullCmpData.size());
        if (request.params.topK > 0) {
            response.total = std::min(request.params.topK, safeSize);
        } else {
            response.total = safeSize;
        }
        response.datas = GetFixNumDiffCmpData(fullCmpData, request.params, response.total);
        return true;
    }

    bool QueryOpDetailInfoHandler::HandleDetailDataRequest(OperatorDetailInfoRequest &request,
                                                           OperatorDetailInfoResponse &response)
    {
        std::string rankId = Summary::VirtualSummaryDataBase::GetFileIdFromCombinationId(request.params.rankId);
        auto database = Timeline::DataBaseManager::Instance().GetSummaryDatabase(rankId);
        if (!database->QueryOperatorDetailInfo(request.params, response)) {
            ServerLog::Error("[Operator]Failed to query detail Info by rankId");
            return false;
        }
        return true;
    }

    void QueryOpDetailInfoHandler::SortDataBynameAndStartTime(std::vector<Protocol::OperatorDetailInfoRes> &baseDbData,
                                                              std::vector<Protocol::OperatorDetailInfoRes> &cmpDbData)
    {
        sort(baseDbData.begin(), baseDbData.end(), [](OperatorDetailInfoRes &a, OperatorDetailInfoRes &b) {
            if (a.name != b.name) {
                return a.name < b.name;
            } else {
                return a.startTime < b.startTime;
            }
        });
        sort(cmpDbData.begin(), cmpDbData.end(), [](OperatorDetailInfoRes &a, OperatorDetailInfoRes &b) {
            if (a.name != b.name) {
                return a.name < b.name;
            } else {
                return a.startTime < b.startTime;
            }
        });
    }

    std::vector<Protocol::OperatorDetailCmpInfoRes> QueryOpDetailInfoHandler::GetCmpDataVec(
        std::vector<Protocol::OperatorDetailInfoRes> &baseDbData,
        std::vector<Protocol::OperatorDetailInfoRes> &cmpDbData)
    {
        SortDataBynameAndStartTime(baseDbData, cmpDbData);
        std::vector<Protocol::OperatorDetailInfoRes>::iterator baseIter = baseDbData.begin();
        std::vector<Protocol::OperatorDetailInfoRes>::iterator cmpIter = cmpDbData.begin();
        std::vector<Protocol::OperatorDetailCmpInfoRes> datailData;

        while (baseIter != baseDbData.end() && cmpIter != cmpDbData.end()) {
            while (baseIter != baseDbData.end() && cmpIter != cmpDbData.end() &&
                  baseIter->name == cmpIter->name) {
                OperatorDetailCmpInfoRes tmp;
                tmp.baseline = *baseIter;
                tmp.compare = *cmpIter;
                datailData.emplace_back(tmp);
                baseIter++;
                cmpIter++;
            }
            if (baseIter == baseDbData.end() || cmpIter == cmpDbData.end()) {
                break;
            }
            while (baseIter != baseDbData.end() && baseIter->name < cmpIter->name) {
                OperatorDetailCmpInfoRes tmp;
                tmp.baseline = *baseIter;
                datailData.emplace_back(tmp);
                baseIter++;
            }
            if (baseIter == baseDbData.end() || cmpIter == cmpDbData.end()) {
                break;
            }
            while (cmpIter != cmpDbData.end() && cmpIter->name < baseIter->name) {
                OperatorDetailCmpInfoRes tmp;
                tmp.baseline = *cmpIter;
                datailData.emplace_back(tmp);
                cmpIter++;
            }
        }
        while (baseIter != baseDbData.end()) {
            OperatorDetailCmpInfoRes tmp;
            tmp.baseline = *baseIter;
            datailData.emplace_back(tmp);
            baseIter++;
        }
        while (cmpIter != cmpDbData.end()) {
            OperatorDetailCmpInfoRes tmp;
            tmp.baseline = *cmpIter;
            datailData.emplace_back(tmp);
            cmpIter++;
        }
        return datailData;
    }

    void QueryOpDetailInfoHandler::FromatDatailData(Protocol::OperatorDetailCmpInfoRes &data)
    {
        if (data.compare.duration == DOUBLE_MIN_VALUE) {
            data.diff.name = data.baseline.name;
        } else if (data.baseline.duration == DOUBLE_MIN_VALUE) {
            data.diff.name = data.compare.name;
        } else {
            data.diff.rankId = data.compare.rankId;
            data.diff.stepId = data.compare.stepId;
            data.diff.name = data.compare.name;
            data.diff.type = data.compare.type + "->" + data.baseline.type;
            data.diff.accCore = data.compare.accCore + "->" + data.baseline.accCore;
            data.diff.startTime = std::to_string(NumberUtil::StringToDouble(data.compare.startTime) -
                                  NumberUtil::StringToDouble(data.baseline.startTime));
            data.diff.duration = NumberUtil::Sub(data.compare.duration, data.baseline.duration);
            data.diff.waitTime = NumberUtil::Sub(data.compare.waitTime, data.baseline.waitTime);
            data.diff.blockDim = data.compare.blockDim - data.baseline.blockDim;
            data.diff.inputShape = data.compare.inputShape + "->" + data.baseline.inputShape;
            data.diff.inputType = data.compare.inputType + "->" + data.baseline.inputType;
            data.diff.inputFormat = data.compare.inputFormat + "->" + data.baseline.inputFormat;
            data.diff.outputShape = data.compare.outputShape + "->" + data.baseline.outputShape;
            data.diff.outputType = data.compare.outputType + "->" + data.baseline.outputType;
            data.diff.outputFormat = data.compare.outputFormat + "->" + data.baseline.outputFormat;
        }
    }

    std::vector<Protocol::OperatorDetailCmpInfoRes> QueryOpDetailInfoHandler::GetFixNumDiffCmpData(
        std::vector<Protocol::OperatorDetailCmpInfoRes> &datailData,
        Protocol::OperatorStatisticReqParams &reqParams,
        const int64_t total)
    {
        if (datailData.empty()) {
            return datailData;
        }
        for (auto &data: datailData) {
            FromatDatailData(data);
        }
        std::string dbOrderBy = reqParams.orderBy;
        // 对差值排序
        const std::string order = reqParams.order;
        std::sort(datailData.begin(), datailData.end(), [&order, &dbOrderBy](Protocol::OperatorDetailCmpInfoRes &a,
                                                                           Protocol::OperatorDetailCmpInfoRes &b) {
            return StaticCmp(a, b, order, dbOrderBy);
        });

        std::vector<Protocol::OperatorDetailCmpInfoRes> topKDetailData;
        topKDetailData.assign(datailData.begin(), datailData.begin() + total);

        // 截取需要的部分 （偏移量） 到 （偏移量 + limit - 1） pageSize 默认是10条，此处防止除零操作
        uint64_t dataSize = topKDetailData.size();
        uint64_t pageSize = (reqParams.pageSize == 0 ? 10 : reqParams.pageSize);
        uint64_t offset = pageSize * (reqParams.current - 1);
        if (offset >= dataSize) {
            offset = dataSize -
                     ((dataSize % pageSize) == 0 ? pageSize : (dataSize % pageSize));
        }
        std::vector<Protocol::OperatorDetailCmpInfoRes>::const_iterator start = topKDetailData.begin() + offset;
        std::vector<Protocol::OperatorDetailCmpInfoRes>::const_iterator end = topKDetailData.begin() +
            std::min(offset + pageSize, dataSize);
        std::vector<Protocol::OperatorDetailCmpInfoRes> result;
        result.assign(start, end);
        return result;
    }
}
