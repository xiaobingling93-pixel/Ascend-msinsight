/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "pch.h"
#include "DataBaseManager.h"
#include "OperatorProtocolRequest.h"
#include "WsSessionManager.h"
#include "OperatorProtocol.h"
#include "QueryOpDetailInfoHandler.h"

namespace Dic::Module::Operator {
    using namespace Dic::Server;

    void QueryOpDetailInfoHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
    {
        OperatorDetailInfoRequest &request = dynamic_cast<OperatorDetailInfoRequest &>(*requestPtr);
        WsSession &session = *WsSessionManager::Instance().GetSession();
        std::unique_ptr<OperatorDetailInfoResponse> responsePtr = std::make_unique<OperatorDetailInfoResponse>();
        OperatorDetailInfoResponse &response = *responsePtr;
        bool rst = false;
        std::string errorMsg;
        if (request.params.CommonCheck(errorMsg)) {
            rst = request.params.isCompare ?
                HandleDetailDataRequest(request, dynamic_cast<OperatorDetailInfoResponse &>(*responsePtr)) :
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
            return false;
        }
        std::vector<Protocol::OperatorDetailCmpInfoRes> res;
        res = CalCompareInfo(response.total, cmpRes, cmpRes, request.params.pageSize, request.params.current);
        response.datas = res;
        return true;
    }

    bool QueryOpDetailInfoHandler::HandleDetailDataRequest(OperatorDetailInfoRequest &request,
                                                           OperatorDetailInfoResponse &response)
    {
        std::string rankId = Summary::VirtualSummaryDataBase::GetFileIdFromCombinationId(request.params.rankId);
        auto database = Timeline::DataBaseManager::Instance().GetSummaryDatabase(rankId);
        if (!database->QueryOperatorDetailInfo(request.params, response)) {
            ServerLog::Error("[Operator]Failed to query Statistic Info, RankId = ", rankId);
            return false;
        }
        return true;
    }

    std::string QueryOpDetailInfoHandler::GetGroup(OperatorDetailInfoRes &data)
    {
        return data.stepId + data.name + data.type + data.accCore + data.inputShape +
               data.inputType + data.inputFormat + data.outputShape + data.outputType +
               data.outputFormat;
    }

    std::vector<Protocol::OperatorDetailCmpInfoRes> QueryOpDetailInfoHandler::CalCompareInfo(int64_t &total,
        std::vector<Protocol::OperatorDetailInfoRes> &baseDbData,
        std::vector<Protocol::OperatorDetailInfoRes> &cmpDbData, int64_t pageSize, int64_t current)
    {
        std::set<std::string> infoKey;
        std::multimap<std::string, Protocol::OperatorDetailInfoRes> multiGroupBaselineMap;
        std::multimap<std::string, Protocol::OperatorDetailInfoRes> multiGroupCompareMap;
        std::vector<Protocol::OperatorDetailCmpInfoRes> datailData;
        // 处理从sql获取的base 和 cmp 数据放在map里
        ProcessDataToMuiMap(cmpDbData, infoKey, multiGroupBaselineMap);
        ProcessDataToMuiMap(baseDbData, infoKey, multiGroupCompareMap);

        for (auto it = infoKey.begin(); it != infoKey.end(); ++it) {
            std::multimap<std::string, OperatorDetailInfoRes>::iterator baseIter = multiGroupBaselineMap.find(*it);
            std::multimap<std::string, OperatorDetailInfoRes>::iterator cmpIter = multiGroupCompareMap.find(*it);
            int baseCount = multiGroupBaselineMap.count(*it);
            int cmpCount = multiGroupCompareMap.count(*it);
            int count = std::min(baseCount, cmpCount);
            int idx = 0;
            for (int idx = 0; idx < count; ++idx, ++baseIter, ++cmpIter) {
                OperatorDetailCmpInfoRes tmpInfo;
                tmpInfo.diff = cmpIter->second;
                tmpInfo.compare = cmpIter->second;
                tmpInfo.baseline = baseIter->second;
                datailData.emplace_back(tmpInfo);
            }
            // 剩余数据处理
            while (idx < baseCount) {
                OperatorDetailCmpInfoRes tmpInfo;
                tmpInfo.diff = baseIter->second;
                tmpInfo.baseline = baseIter->second;
                datailData.emplace_back(tmpInfo);
                ++baseIter;
                ++idx;
            }
            while (idx < cmpCount) {
                OperatorDetailCmpInfoRes tmpInfo;
                tmpInfo.diff = cmpIter->second;
                tmpInfo.compare = cmpIter->second;
                datailData.emplace_back(tmpInfo);
                ++cmpIter;
                ++idx;
            }
        }
        total = datailData.size();
        return GetFixNumDiffCmpData(datailData, pageSize, current);
    }

    void QueryOpDetailInfoHandler::ProcessDataToMuiMap(std::vector<Protocol::OperatorDetailInfoRes> datFromDb,
        std::set<std::string> infoKey, std::multimap<std::string, Protocol::OperatorDetailInfoRes> multiDataMap)
    {
        std::string group;
        for (auto &data : datFromDb) {
            group = GetGroup(data);
            if (group.empty()) {
                continue;
            }
            infoKey.insert(group);
            multiDataMap.insert(std::make_pair(group, data));
        }
    }

    std::vector<Protocol::OperatorDetailCmpInfoRes> QueryOpDetailInfoHandler::GetFixNumDiffCmpData(
        std::vector<Protocol::OperatorDetailCmpInfoRes> &datailData, const int64_t paraPageSize,
        const int64_t current)
    {
        if (datailData.empty()) {
            return datailData;
        }
        for (auto &data: datailData) {
            if (data.compare.startTime.empty() || data.baseline.startTime.empty()) {
                data.diff.startTime = "-";
                continue;
            }
            data.diff.startTime = std::to_string(StringUtil::StringToDouble(data.compare.startTime) -
                                  StringUtil::StringToDouble(data.baseline.startTime));
            data.diff.duration = data.compare.duration - data.baseline.duration;
            data.diff.waitTime = data.compare.waitTime - data.baseline.waitTime;
            data.diff.blockDim = data.compare.blockDim - data.baseline.waitTime;
        }
        // 对差值排序
        std::sort(datailData.begin(), datailData.end(), [](const auto& a, const auto& b) {
                return a.diff.duration > b.diff.duration;
            });

        // 截取需要的部分 （偏移量） 到 （偏移量 + limit - 1） pageSize 默认是10条，此处防止除零操作
        uint64_t pageSize = (paraPageSize == 0 ? 10 : paraPageSize);
        uint64_t offset = pageSize * (current - 1);
        if (offset >= datailData.size()) {
            offset = datailData.size() -
                     ((datailData.size() % pageSize) == 0 ? pageSize : (datailData.size() % pageSize));
        }
        std::vector<Protocol::OperatorDetailCmpInfoRes>::const_iterator start = datailData.begin() + offset;
        std::vector<Protocol::OperatorDetailCmpInfoRes>::const_iterator end = datailData.begin() +
            std::min(offset + pageSize - 1, datailData.size() - 1);
        std::vector<Protocol::OperatorDetailCmpInfoRes> result;
        result.assign(start, end);
        return result;
    }

}
