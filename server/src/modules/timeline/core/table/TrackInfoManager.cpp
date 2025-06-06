//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//
#include "pch.h"
#include "DataBaseManager.h"
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
    std::string deviceId = GetDeviceId(cardId);
    std::string host = GetHost(cardId);
    trackInfoMap[maxTrackId] = {host, cardId, rankId, deviceId, pid, tid, maxTrackId};
    return maxTrackId;
}

void TrackInfoManager::Reset()
{
    std::unique_lock<std::mutex> lock(trackMutex);
    trackIdMap.clear();
    hostMap.clear();
    hostCardIdMap.clear();
    trackInfoMap.clear();
    maxTrackId = 0;
    deviceMap.clear();
    deviceIdToRankIdMap.clear();
    fileIdToRankListMap.clear();
    fileIdToClusterMap.clear();
    clusterDbToFileIdMap.clear();
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

std::string TrackInfoManager::GetDeviceId(const std::string &cardId)
{
    std::string rankId = GetRankId(cardId);
    if (deviceMap.count(cardId) == 0) {
        return rankId;
    }
    if (deviceMap.at(cardId).count(rankId) == 0) {
        return rankId;
    }
    return deviceMap.at(cardId).at(rankId);
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
        Server::ServerLog::Warn("Failed to query track info, track id is: ", trackId);
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
        std::string deviceId = GetDeviceId(fileId);
        std::string host = GetHost(fileId);
        trackInfoMap[key] = {host, fileId, rankId, deviceId, item.second, item.first, key};
        maxTrackId = std::max(maxTrackId, key);
    }
}

void TrackInfoManager::UpdateDeviceMap(const std::string &cardId,
    const std::unordered_map<std::string, std::string> rankAndDeviceMap)
{
    std::unique_lock<std::mutex> lock(trackMutex);
    deviceMap[cardId] = rankAndDeviceMap;
    std::string host = hostMap[cardId];
    for (const auto &item: rankAndDeviceMap) {
        std::string key = host + item.second;
        deviceIdToRankIdMap[key] = item.first;
    }
}

void TrackInfoManager::UpdateHostCardId(const std::string &cardId, const std::string &hostCardId)
{
    std::unique_lock<std::mutex> lock(trackMutex);
    hostCardIdMap[cardId] = hostCardId;
}

std::string TrackInfoManager::GetHostCardId(const std::string &deviceCardId)
{
    std::unique_lock<std::mutex> lock(trackMutex);
    auto it = hostCardIdMap.find(deviceCardId);
    if (it == hostCardIdMap.end()) {
        return deviceCardId;
    }
    return it->second;
}

std::string TrackInfoManager::GetRankId(const std::string &host, const std::string &deviceId)
{
    std::unique_lock<std::mutex> lock(trackMutex);
    std::string key = host + deviceId;
    auto it = deviceIdToRankIdMap.find(key);
    if (it != deviceIdToRankIdMap.end()) {
        return it->second;
    }
    Server::ServerLog::Warn("Failed to query rank id by device id, device id is: ", deviceId);
    return {};
}

void TrackInfoManager::UpdateClusterDbToFileIdMap(const std::string &clusterDb, const std::string &fileId)
{
    std::unique_lock<std::mutex> lock(trackMutex);
    clusterDbToFileIdMap[clusterDb].insert(fileId);
}

std::map<std::string, std::string> TrackInfoManager::GetRankIdToFileIdByClusterDb(const std::string &clusterDb)
{
    std::unique_lock<std::mutex> lock(trackMutex);
    // 根据集群db获取该集群下所有的fileId
    auto it = clusterDbToFileIdMap.find(clusterDb);
    if (it == clusterDbToFileIdMap.end()) {
        return {};
    }
    // 再根据fileId找到对应的rankId（目前单host多device没有集群）
    std::map<std::string, std::string> res;
    for (const auto &item: clusterDbToFileIdMap[clusterDb]) {
        res[DataBaseManager::Instance().GetRankIdByFileId(item)] = item;
    }
    return res;
}

std::vector<RankInfo> TrackInfoManager::GetRankListByFileId(const std::string &fileId, const std::string &rankId)
{
    std::lock_guard lock(trackMutex);
    return fileIdToRankListMap[fileId + rankId];
}

void TrackInfoManager::SetRankListByFileId(const std::string &fileId, const RankInfo& rankInfo)
{
    std::lock_guard lock(trackMutex);
    fileIdToRankListMap[fileId + rankInfo.rankId].push_back(rankInfo);
}

std::string TrackInfoManager::GetClusterByFileId(const std::string &fileId)
{
    std::lock_guard lock(trackMutex);
    return fileIdToClusterMap[fileId];
}

void TrackInfoManager::SetClusterByFileId(const std::string &fileId, const std::string &cluster)
{
    std::lock_guard lock(trackMutex);
    fileIdToClusterMap[fileId] = cluster;
}

std::string TrackInfoManager::GetFileIdByClusterDbAndRankId(const std::string &clusterDb, const std::string &rankId)
{
    std::map<std::string, std::string> rankIdToFileIdMap = GetRankIdToFileIdByClusterDb(clusterDb);
    const size_t splitSizeWithHost = 2;
    for (auto &item: rankIdToFileIdMap) {
        auto splitList = StringUtil::Split(item.first, " ");
        // rankId没有host的情况
        if (splitList.size() == 1 && splitList[0] == rankId) {
            return item.second;
        }
        // rankId有host的情况
        if (splitList.size() == splitSizeWithHost && splitList[1] == rankId) {
            return item.second;
        }
    }
    return "";
}
}
