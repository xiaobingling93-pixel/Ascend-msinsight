/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "BinFileGenerator.h"

namespace Dic::Module::Source::Test {
void BinFileGenerator::Generate(const std::string& outputPath)
{
    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile) {
        return;
    }
    for (const auto &item: dataBlocks) {
        item->Write2File(outFile);
    }
    outFile.close();
}

}

using namespace Dic::Module::Source::Test;