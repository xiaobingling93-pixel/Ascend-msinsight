/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "IECurveHandler.h"
#include "IEProtocolResquest.h"
#include "IEProtocolResponse.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
namespace Dic::Module::IE {
using namespace Dic;
using namespace Dic::Server;
bool IECurveHandler::HandleRequest(std::unique_ptr<Dic::Protocol::Request> requestPtr)
{
    auto& request = dynamic_cast<IEUsageViewParamsRequest&>(*requestPtr);
    std::unique_ptr<IEUsageViewResponse> responsePtr = std::make_unique<IEUsageViewResponse>();
    IEUsageViewResponse& response = *responsePtr;
    SetBaseResponse(request, response);
    if (!request.params.type.empty()) {
        auto atts = repo->QueryTableInfoByName(request.params.rankId, request.params.type);
        if (atts.size() <= 1) {
            SendResponse(std::move(responsePtr), true);
            return true;
        }
        QueryDatasByCols(request, response, atts);
    }
    SendResponse(std::move(responsePtr), true);
    return true;
}

void IECurveHandler::QueryDatasByCols(const IEUsageViewParamsRequest& request, IEUsageViewResponse& response,
                                      std::vector<ColumnAtt>& atts)
{
    std::string rankId = request.params.rankId;
    std::string tableName = request.params.type;
    auto datas = repo->QueryDataByColumn(rankId, tableName, atts);
    std::vector<std::string> curline(atts.size());
    for (auto& item : datas) {
        std::vector<std::string> line;
        for (const auto& att : atts) {
            line.emplace_back(item[att.key]);
        }
        for (size_t i = 0; i < atts.size(); ++i) {
            if (curline[i] != line[i]) {
                response.data.lines.emplace_back(line);
                curline = std::move(line);
                break;
            }
        }
    }
    for (const auto& att : atts) {
        response.data.legends.emplace_back(att.key);
    }
}
}  // namespace Dic::Module::IE
