/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_BINFILEGENERATOR_H
#define PROFILER_SERVER_BINFILEGENERATOR_H

#include <utility>

#include <istream>
#include <vector>
#include <memory>
#include "DataBlock.h"

namespace Dic::Module::Source::Test {

class BinFileGenerator {
public:
    void AddDataBlock(std::unique_ptr<DataBlock> blockPtr)
    {
        dataBlocks.push_back(std::move(blockPtr));
    }

    void Generate(const std::string &outputPath);

private:
    std::vector<std::unique_ptr<DataBlock>> dataBlocks;
};

}

#endif // PROFILER_SERVER_BINFILEGENERATOR_H
