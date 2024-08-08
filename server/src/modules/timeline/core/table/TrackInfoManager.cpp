//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//
#include "pch.h"
#include "TrackInfoManager.h"
namespace Dic::Module::Timeline {
uint64_t TrackInfoManager::GetTrackId(const std::string &cardId, const std::string &pid, const std::string &tid)
{
    std::unique_lock<std::mutex> lock(trackMutex);
    auto item = std::make_pair(pid, tid);
    if (trackIdMap[cardId].count(item) > 0) {
        return trackIdMap[cardId].at(item);
    }
    if (maxTrackId == UINT64_MAX) {
        maxTrackId = 0;
    }
    trackIdMap[cardId].emplace(item, ++maxTrackId);
    std::string rankId = GetRankId(cardId);
    std::string host = GetHost(cardId);
    trackInfoMap[maxTrackId] = {host, cardId, rankId, pid, tid, maxTrackId};
    return maxTrackId;
}

void TrackInfoManager::Reset()
{
    std::unique_lock<std::mutex> lock(trackMutex);
    trackIdMap.clear();
    hostMap.clear();
    trackInfoMap.clear();
    maxTrackId = 0;
}

void TrackInfoManager::UpdateHost(const std::string &cardId, const std::string &host)
{
    std::unique_lock<std::mutex> lock(trackMutex);
    hostMap[cardId] = host;
}

std::string TrackInfoManager::GetRankId(const std::string &cardId)
{
    if (hostMap.count(cardId) == 0 || !StringUtil::StartWith(cardId, hostMap.at(cardId))) {
        return cardId;
    }
    return cardId.substr(hostMap.at(cardId).length());
}

std::string TrackInfoManager::GetHost(const std::string &cardId)
{
    if (hostMap.count(cardId) == 0) {
        return "";
    }
    return hostMap.at(cardId);
}

bool TrackInfoManager::GetTrackInfo(uint64_t trackId, TrackInfo &trackInfo)
{
    std::unique_lock<std::mutex> lock(trackMutex);
    if (trackInfoMap.count(trackId) == 0) {
        return false;
    }
    trackInfo = trackInfoMap.at(trackId);
    return true;
}

void TrackInfoManager::UpdateTrackIdMap(const std::string &fileId,
    const std::map<uint64_t, std::pair<std::string, std::string>> &threadMap)
{
    std::unique_lock<std::mutex> lock(trackMutex);
    for (auto [key, item] : threadMap) {
        std::pair<std::string, std::string> tmp = { item.second, item.first };
        trackIdMap[fileId].emplace(tmp, key);
        std::string rankId = GetRankId(fileId);
        std::string host = GetHost(fileId);
        trackInfoMap[key] = {host, fileId, rankId, item.second, item.first, key};
        maxTrackId = std::max(maxTrackId, key);
    }
}
}
