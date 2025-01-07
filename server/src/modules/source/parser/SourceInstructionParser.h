/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SOURCEINSTRUCTIONPARSER_H
#define PROFILER_SERVER_SOURCEINSTRUCTIONPARSER_H

#include "SourceProtocol.h"

namespace Dic::Module::Source {
struct SourceFileLine {
    int line;
    std::vector<float> cycles;
    std::vector<int> instructionsExecuted;
    std::vector<std::pair<std::string, std::string>> addressRange;
};

struct ColumDataType {
    enum Type : int {
        STRING = 0,
        INT,
        FLOAT
    };
};

struct SourceFileInstruction {
    std::map<std::string, std::vector<std::string>> stringColumnMap; // column name to data
    std::map<std::string, std::vector<int>> intColumnMap;
    std::map<std::string, std::vector<float>> floatColumnMap;
};

class SourceInstructionParser {
public:
    SourceInstructionParser() = default;
    bool ConvertToData(std::string &filePath, std::vector<Position> &sourceFilePos, std::vector<Position> &apiFilePos,
                       std::vector<Position> &apiInstrPosArray);
    std::vector<std::string> GetCoreList();
    std::vector<std::string> GetSourceList();
    std::vector<SourceFileLine> GetApiLinesByCoreAndSource(const std::string &core, const std::string &sourceName);
    std::string GetInstr(std::string &filePath);
    std::vector<SourceFileInstruction> GetInstructionsByCoreName(std::string &coreName);
    std::string GetSourceByName(std::string &sourceName, std::string &filePath);
    void Reset();

protected:
    void ConvertApiInstr(const std::string &jsonStr);
    void ConvertApiInstrNew(const std::string &jsonStr);
    void ParseInstruction(rapidjson::Value &instr);
    template <typename T>
    void ProcessColumnDataArray(const Value& value, std::vector<T>& columnDataList);
    template <typename T>
    void ProcessColumnData(const Value& value, std::vector<T>& columnDataList);
    void ConvertApiFile(const std::string &jsonStr);
    std::map<std::string, std::vector<SourceFileLine>> ConvertToFileMap(rapidjson::Value &fileArray);
    std::vector<SourceFileLine> ConvertToLineArray(rapidjson::Value &lineArray);
    std::vector<SourceFileInstruction>& GetInstructionList()
    {
        return instructionList;
    }

private:
    const static uint16_t filePathLengthConst = 4096;
    const static uint16_t addressRangeSize = 2;
    std::map<std::string, Position> sourceFiles;
    std::map<std::string, std::vector<SourceFileLine>> apiFiles; // source file name to lines
    std::vector<std::string> apiCores;
    std::vector<SourceFileInstruction> instructionList;
    std::map<std::string, int> instructionColumnTypeMap; // column name to data type
    Position apiInstrPos{};
};

}

#endif // PROFILER_SERVER_SOURCEINSTRUCTIONPARSER_H
