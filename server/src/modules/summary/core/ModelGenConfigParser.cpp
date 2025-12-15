/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "ModelGenConfigParser.h"
#include "FileReader.h"
#include "JsonUtil.h"
#include "SummaryErrorManager.h"

namespace Dic::Module::Summary {
ModelGenConfig ModelGenConfigParser::ParserModelGenConfigByFilePath(const std::string &filePath, std::string &error)
{
    FileReader reader;
    std::string fileContext = reader.ReadJsonArray(filePath, 0, 0);
    ModelGenConfig res;
    if (fileContext.empty()) {
        error = "Fail to read model gen config file, the file is empty.";
        SetSummaryError(ErrorCode::READ_MODEL_GEN_CONFIG_FILE_FAILED);
        return res;
    }
    try {
        auto configJsonOpt = JsonUtil::TryParse(fileContext, error);
        if (!error.empty()) {
            return res;
        }
        if (!configJsonOpt.value().IsObject()) {
            error = "Fail to parser config file, data in wrong format.";
            SetSummaryError(ErrorCode::READ_MODEL_GEN_CONFIG_FILE_FAILED);
            return res;
        }
        int moeLayer = JsonUtil::GetInteger(configJsonOpt.value(), "num_moe_layers");
        if (moeLayer < 0) {
            error = "Fail to read model gen config file, the number of moe layer can't less than zero.";
            SetSummaryError(ErrorCode::READ_MODEL_GEN_CONFIG_FILE_FAILED);
            return res;
        }
        res.moeLayer = static_cast<uint64_t>(moeLayer);
        return res;
    } catch (const std::exception &e) {
        error = "Fail to parser meta data file context. Except: " + std::string(e.what());
        SetSummaryError(ErrorCode::PARSER_META_DATA_FILE_CONTEXT_FAILED);
        return res;
    }
}
}