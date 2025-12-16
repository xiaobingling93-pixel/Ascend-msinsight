/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
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

#ifndef PROFILER_SERVER_SOURCEINSTRUCTIONPARSER_H
#define PROFILER_SERVER_SOURCEINSTRUCTIONPARSER_H
#include <map>
#include <unordered_map>
#include <vector>
#include "SourceProtocol.h"
#include "SourceDefs.h"


namespace Dic::Module::Source {
struct SourceFileLine {
    int line;
    std::vector<float> cycles;
    std::vector<int> instructionsExecuted;
    std::vector<std::pair<std::string, std::string>> addressRange;
};

struct SourceApiInstruction {
    std::string address;
    std::string ascendCInnerCode;
    std::vector<int> cycles;
    std::vector<int> instructionsExecuted;
    std::string pipe;
    std::string source;
    std::vector<int> theoreticalStallCycles;
    std::vector<int> realStallCycles;
};

struct DynamicColumn {
    std::unordered_map<std::string, std::vector<std::string>> stringColumnMap; // column name to data
    std::unordered_map<std::string, std::vector<int>> intColumnMap;
    std::unordered_map<std::string, std::vector<float>> floatColumnMap;
};

struct SourceFileInstructionDynamicCol : public DynamicColumn {
};

struct SourceFileLineDynamicCol : public DynamicColumn {
    std::vector<std::pair<std::string, std::string>> addressRange;
};

class SourceInstructionParser {
public:
    SourceInstructionParser() = default;
    bool ConvertToData(std::string &filePath, std::vector<Position> &sourceFilePos, std::vector<Position> &apiFilePos,
                       std::vector<Position> &apiInstrPosArray);
    std::vector<std::string> GetCoreList();
    std::vector<std::string> GetSourceList();
    std::vector<SourceFileLine> GetApiLinesByCoreAndSource(const std::string &core, const std::string &sourceName);
    std::vector<SourceFileLineDynamicCol> GetApiLinesDynamic(const std::string &core, const std::string &sourceName);
    std::string GetInstr(std::string &filePath);
    std::vector<SourceApiInstruction> GetInstructions(std::string &coreName);
    std::vector<SourceFileInstructionDynamicCol> GetInstrDynamic(std::string &coreName);
    std::string GetSourceByName(std::string &sourceName, std::string &filePath);
    std::map<std::string, int> GetInstructionColumnTypeMap() const;
    std::map<std::string, int> GetSourceLineColumnTypeMap() const;

    void Reset();

protected:
    void ConvertApiInstr(const std::string &jsonStr);
    void ConvertApiInstrDynamic(const std::string &jsonStr);
    void ParseInstruction(rapidjson::Value &instr);
    template <typename T>
    void ProcessColumnDataArray(const Value& value, std::vector<T>& columnDataList);
    template <typename T>
    void ProcessColumnData(const Value& value, std::vector<T>& columnDataList);
    void ConvertApiFile(const std::string &jsonStr);
    void ConvertApiFileDynamic(const std::string &jsonStr);
    void ParseFile(rapidjson::Value &file);
    void ParseSourceLineAddressRange(const Value &line, SourceFileLineDynamicCol &sourceFileLine);
    std::map<std::string, std::vector<SourceFileLine>> ConvertToFileMap(rapidjson::Value &fileArray);
    std::vector<SourceFileLine> ConvertToLineArray(rapidjson::Value &lineArray);
    template<typename T>
    void GetValueInTargetCore(const std::unordered_map<std::string, std::vector<T>> &sourceMap,
                              std::unordered_map<std::string, std::vector<T>> &targetMap, size_t index);
    std::vector<SourceFileInstructionDynamicCol>& GetInstructionList()
    {
        return instructionList;
    }
    std::unordered_map<std::string, std::vector<SourceFileLineDynamicCol>>& GetSourceLinesMap()
    {
        return sourceLinesMap;
    }
    std::vector<SourceApiInstruction>& GetApiInstructionList()
    {
        return apiInstructionList;
    }
    std::map<std::string, std::vector<SourceFileLine>>& GetApiFiles()
    {
        return apiFiles;
    }

private:
    const static uint16_t filePathLengthConst = 4096;
    const static uint16_t addressRangeSize = 2;
    std::map<std::string, Position> sourceFiles;
    std::map<std::string, std::vector<SourceFileLine>> apiFiles; // source file name to lines
    std::vector<SourceApiInstruction> apiInstructionList;
    std::vector<std::string> apiCores;
    std::vector<SourceFileInstructionDynamicCol> instructionList;
    std::map<std::string, int> instructionColumnTypeMap; // instruction column name to data type
    std::unordered_map<std::string, std::vector<SourceFileLineDynamicCol>> sourceLinesMap; // source file name to lines
    std::map<std::string, int> sourceLineColumnTypeMap; // source line column name to data type
    Position apiInstrPos{};
};

}

#endif // PROFILER_SERVER_SOURCEINSTRUCTIONPARSER_H
