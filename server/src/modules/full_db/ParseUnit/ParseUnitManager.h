/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_PARSEUNITMANAGER_H
#define PROFILER_SERVER_PARSEUNITMANAGER_H

#include <map>
#include "AbstractParseUnit.h"
namespace Dic::Module::FullDb {
class ParseUnitManager {
public:
    using Creator = std::function<std::unique_ptr<AbstractParseUnit>()>;
    static ParseUnitManager &Instance();

    ParseUnitManager(const ParseUnitManager &) = delete;
    ParseUnitManager &operator=(const ParseUnitManager &) = delete;
    ParseUnitManager(ParseUnitManager &&) = delete;
    ParseUnitManager &operator=(ParseUnitManager &&) = delete;

    void ExecuteAll(const ParseUnitParams &params);
    void RegisterUnit(const std::string& name, Creator unit);

private:
    ParseUnitManager() = default;
    ~ParseUnitManager() = default;
    std::map<std::string, Creator> unitMap;
};

template <typename T>
class ParseUnitRegistrar {
public:
    explicit ParseUnitRegistrar(const std::string &name)
    {
        ParseUnitManager::Instance().RegisterUnit(name, []() {
            return std::make_unique<T>();
        });
    }
};
}
#endif // PROFILER_SERVER_PARSEUNITMANAGER_H
