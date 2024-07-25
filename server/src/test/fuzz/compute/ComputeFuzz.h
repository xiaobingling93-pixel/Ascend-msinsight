/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef COMPUTE_FUZZ_H
#define COMPUTE_FUZZ_H
#include <string>
#include "../../../../third_party/googletest/googletest/include/gtest/gtest.h"
#include "../../../../third_party/secodefuzz/Secodefuzz/secodeFuzz.h"

extern int g_fuzzRunTime;

struct BinaryBlockHeader {
    uint64_t contentSize = 0;
    uint8_t type = 0;
    uint8_t padding = 0;
    uint16_t reverse = 0x5a5a;
};

struct BinaryBlockHeader4File {
    BinaryBlockHeader binaryBlockHeader{};
    char filePath[4096] = {0};
};
#pragma pack()

const int INTEGER_BYTES = 4;

inline void GetFilePadding(BinaryBlockHeader &binaryBlockHeader, const uint64_t &sizeContent, std::string &buffer)
{
    if (sizeContent % INTEGER_BYTES != 0) {
        binaryBlockHeader.padding = INTEGER_BYTES - (sizeContent % INTEGER_BYTES);
        buffer.resize(sizeContent + binaryBlockHeader.padding, 0);
    }
}

// namespace
#endif // COMPUTE_FUZZ_H
