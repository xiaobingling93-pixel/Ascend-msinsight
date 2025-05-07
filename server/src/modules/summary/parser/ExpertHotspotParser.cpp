/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "ExpertHotspotParser.h"
#include "SafeFile.h"

namespace Dic::Module::Summary {
    bool ExpertHotspotParser::Parse(const std::string &filePath, const std::string &version)
    {
        if (db == nullptr) {
            return false;
        }
        // 解析文件名，获取模型阶段和rankId
        auto searchRes = RegexUtil::RegexSearch(filePath, expertHotspotFileReg);
        if (!searchRes.has_value() || searchRes.value().size() != regexMatchNumber) {
            return false;
        }

        // 读取文件，读取过程中会校验文件状态
        std::ifstream file = OpenReadFileSafely(filePath);
        std::string line;
        int rankId = NumberUtil::StringToInt(searchRes.value()[2]);
        std::string modelStage = searchRes.value()[1];
        int layer = 0;
        size_t expertCountPerRank = 0;
        while (getline(file, line)) {
            std::vector<std::string> row = StringUtil::StringSplit(line);
            if (expertCountPerRank == 0) {
                // 读取的第一行数据作为每个rank的专家数量
                expertCountPerRank = row.size();
            }
            if (expertCountPerRank != row.size()) {
                Server::ServerLog::Warn("Invalid row size, model type:", modelStage, ", rank id:", rankId,
                                        ", row number:", layer);
            }
            for (size_t i = 0; i < row.size(); ++i) {
                ExpertHotspotStruct expertHotspot{modelStage, rankId, NumberUtil::StringToUnsignedLongLong(row[i]),
                                                  layer, static_cast<int>(i), version};
                db->InsertExpertHotspotDataForCache(expertHotspot);
            }
            layer++;
        }
        modelInfoMap[modelStage].rankNumber++;
        modelInfoMap[modelStage].moeLayer = layer;
        // 单独看热点数据，没有专家id，因此使用专家数量作为专家id的最大值
        modelInfoMap[modelStage].expertNumber += expertCountPerRank;
        return true;
    }

    std::map<std::string, ModelInfo> ExpertHotspotParser::GetModelInfoMap()
    {
        return modelInfoMap;
    }
}

