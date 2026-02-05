/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

#ifndef DATA_INSIGHT_CORE_MODULE_CORE_ACLGRAPH_DEBUG_JSON_FILE_PARSER_H
#define DATA_INSIGHT_CORE_MODULE_CORE_ACLGRAPH_DEBUG_JSON_FILE_PARSER_H

#include "TraceFileParser.h"
#include "TextTraceDatabase.h"

namespace Dic::Module::Timeline {
class ACLGraphDebugJsonFileParser : public TraceFileParser {
public:
    // 构造时注入共享线程池
    explicit ACLGraphDebugJsonFileParser(std::shared_ptr<ThreadPool> threadPool)
    : TraceFileParser(std::move(threadPool)) {}
    ~ACLGraphDebugJsonFileParser() override = default;
protected:
    // 后处理 Hook 函数
    bool PostParse(std::shared_ptr<TextTraceDatabase> db) override;
};
} // end of namespace Dic::Module::Timeline

#endif // DATA_INSIGHT_CORE_MODULE_CORE_ACLGRAPH_DEBUG_JSON_FILE_PARSER_H
