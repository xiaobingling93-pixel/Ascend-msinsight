/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SOURCEFILEPARSER_H
#define PROFILER_SERVER_SOURCEFILEPARSER_H

#include <map>
#include "document.h"
#include "rapidjson.h"
#include "document.h"
#include "FileParser.h"

namespace Dic {
namespace Module {
namespace Source {

struct SourceFileLine {
    int line;
    std::vector<float> cycles;
    std::vector<int> instructionsExecuted;
    std::vector<std::pair<std::string, std::string>> addressRange;
};

enum class DataTypeEnum : int {
    SOURCE = 1,
    TRACE = 2,
    API_FILE = 3,
    API_INSTR = 4,
};

class SourceFileParser : public FileParser {
public:
    const static uint16_t reverseConst = 0x5a5a;
    const static uint16_t filePathLengthConst = 4096;
    const static uint16_t addressRangeSize = 2;

    static SourceFileParser &Instance();

    SourceFileParser();
    ~SourceFileParser() override;

    bool Parse(const std::vector<std::string> &filePaths, const std::string &fileId,
               const std::string &selectedFile) override;
    void Reset() override;
    bool CheckOperatorBinary(const std::string &selectedFilePath);
    std::vector<std::string> GetCoreList();
    std::vector<std::string> GetSourceList();
    std::vector<SourceFileLine> GetApiLinesByCoreAndSource(std::string core, std::string sourceName);
    std::string GetInstr();
    std::string GetSourceByName(std::string sourceName);
    void ConvertToData();
private:
    std::string filePath;
    std::map<int, std::vector<std::pair<int64_t, int64_t>>> dataBlockMap;
    std::map<std::string, std::pair<int64_t, int64_t>> sourceFiles;
    std::map<std::string, std::pair<int64_t, int64_t>> traceFiles;

    std::vector<std::string> apiCores;
    std::map<std::string, std::vector<SourceFileLine>> apiFiles;
    std::pair<int64_t, int64_t> apiInstrPos;

    std::map<std::string, std::vector<SourceFileLine>> ConvertToFileMap(rapidjson::Value &fileArray);
    std::vector<SourceFileLine> ConvertToLineArray(rapidjson::Value &lineArray);
};
} // end of namespace Summary
} // end of namespace Module
} // end of namespace Dic


#endif // PROFILER_SERVER_SOURCEFILEPARSER_H
