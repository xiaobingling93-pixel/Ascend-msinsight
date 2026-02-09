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

#ifndef PROFILER_SERVER_MEMSCOPEPARSER_H
#define PROFILER_SERVER_MEMSCOPEPARSER_H
#include <string>
#include "ThreadPool.h"

namespace Dic::Module {
class MemScopeParser {
public:
    MemScopeParser(const MemScopeParser&) = delete;
    MemScopeParser& operator=(const MemScopeParser&) = delete;
    static MemScopeParser& Instance();
    void Reset() const;
    void AsyncParseMemScopeDbFile(const std::string& dbPath) const;
    static void ParseMemScopeDbTask(const std::string& dbPath);
    static void ParserEnd(const std::string& dbPath, bool result);
    static void ParseCallBack(const std::string& dbPath, bool result, const std::string& msg);
private:
    MemScopeParser();
    ~MemScopeParser();
    std::unique_ptr<ThreadPool> _threadPool;
};
}
#endif //PROFILER_SERVER_MEMSCOPEPARSER_H
