/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "SourceInstructionParser.h"
#include "SafeFile.h"
#include "ServerLog.h"
#include "BinFileParseUtil.h"
#include "JsonUtil.h"

namespace Dic {
namespace Module {
namespace Source {
using namespace Dic::Server;
bool SourceInstructionParser::ConvertToData(std::string &filePath, std::vector<Position> &sourceFilePos,
                                            std::vector<Position> &apiFilePos, std::vector<Position> &apiInstrPosArray)
{
    std::ifstream file = OpenReadFileSafely(filePath, std::ios::binary);
    if (!file) {
        ServerLog::Error("Can't open file, please check file exist or not, file name: ", filePath);
        return false;
    }

    for (auto pos : sourceFilePos) {
        int64_t start = pos.startPos;
        int64_t end = pos.endPos;
        if ((start < 0) || (filePathLengthConst > INT64_MAX - start)) {
            ServerLog::Error(std::string("Start position: ") + std::to_string(start) +
                             std::string(" is illegal at covert to data in source file."));
            return false;
        }
        file.seekg(start, std::ios::beg);

        std::vector<char> filePathBuffer(filePathLengthConst);
        file.read(filePathBuffer.data(), filePathBuffer.size());
        if (!file) {
            ServerLog::Error("Failed to read file path buffer.");
            break;
        }
        std::string sourceFilePath(filePathBuffer.data());
        sourceFiles[sourceFilePath] = {start + filePathLengthConst, end};
    }

    if (!apiFilePos.empty()) {
        Position &pair = apiFilePos.at(0);
        std::string jsonStr = BinFileParseUtil::GetContentStr(file, pair);
        ConvertApiFile(jsonStr);
    }
    if (!apiInstrPosArray.empty()) {
        apiInstrPos = apiInstrPosArray.at(0);
        Position &pair = apiInstrPosArray.at(0);
        std::string jsonStr = BinFileParseUtil::GetContentStr(file, pair);
        ConvertApiInstr(jsonStr);
    }
    file.close();
    return true;
}

/*
json示例
{
  "Cores": [ // 执行算子的计算核，如"core0.cubecore0"，"core0.veccore0"
    string
  ],
"Instructions Dtype": { // 指定列名和数据类型
  // string 0, int 1, float 2
  "Instructions": {
      "Address": 0,
      "Cycles": 1
  }
}
  "Instructions": [
    {
      "Address": string, 			// 指令的偏移地址,如"0x1269f000"
      "AscendC Inner Code": string, // 源代码文件路径和代码行号,如"/home/xxx.cpp:23"
      "Cycles": [ 					// 指令在各个计算核上消耗的时钟周期
        int
      ],
      "Instructions Executed": [ 	// 指令在各个计算核上执行的次数
        int
      ],
      "Pipe": string, 				// 指令所属的指令队列,如"SCALAR"
      "TheoreticalStallCycles": [                    // 预期阻塞时间
        int
       ],
      "Source": string, 				// 指令内容, 如"MOV_XD_IMM XD:X29,IMM"
      "RealStallCycles": [                    // 实际阻塞时间
        int
       ]
    }
  ]
}
 */
void SourceInstructionParser::ConvertApiInstrDynamic(const std::string &jsonStr)
{
    std::string errMsg;
    auto optional = JsonUtil::TryParse(jsonStr, errMsg);
    if (!optional.has_value() || !errMsg.empty()) {
        ServerLog::Error("Parse instr json failed. Error is ", errMsg);
        return;
    }
    auto &d = optional.value();
    // parse column info
    if (!d.HasMember("Instructions Dtype") || !d["Instructions Dtype"].HasMember("Instructions")) {
        return;
    }
    // 遍历dtype中的列对象，获取列名和对应的数据类型
    auto &instructions = d["Instructions Dtype"]["Instructions"];
    for (auto column = instructions.MemberBegin(); column != instructions.MemberEnd(); ++column) {
        std::string columnName = column->name.GetString();
        auto &value = column->value;
        if (!value.IsInt()) {
            continue;
        }
        instructionColumnTypeMap[columnName] = value.GetInt();
    }

    // 根据列信息动态解析Instruction数据
    if (!JsonUtil::IsJsonArray(d, "Instructions")) {
        return;
    }
    for (auto &instr : d["Instructions"].GetArray()) {
        ParseInstruction(instr);
    }
}

void SourceInstructionParser::ParseInstruction(Value &instr)
{
    SourceFileInstructionDynamicCol sourceFileInstruction;
    for (const auto &columnType: instructionColumnTypeMap) {
        std::string columnName = columnType.first;
        int type = columnType.second;
        if (!instr.HasMember(columnName.c_str())) {
            continue;
        }
        // 处理不同数据类型的列
        auto &columData = instr[columnName.c_str()];
        if (type == ColumDataType::STRING) {
            ProcessColumnDataArray<std::string>(columData, sourceFileInstruction.stringColumnMap[columnName]);
        } else if (type == ColumDataType::INT) {
            ProcessColumnDataArray<int>(columData, sourceFileInstruction.intColumnMap[columnName]);
        } else if (type == ColumDataType::FLOAT) {
            ProcessColumnDataArray<float>(columData, sourceFileInstruction.floatColumnMap[columnName]);
        }
    }
    instructionList.emplace_back(std::move(sourceFileInstruction));
}

template <typename T>
void SourceInstructionParser::ProcessColumnDataArray(const Value& value, std::vector<T>& columnDataList)
{
    if (!value.IsArray()) {
        // 如果不是数组，直接处理单一值
        ProcessColumnData(value, columnDataList);
        return;
    }
    // 如果是数组，遍历数组中的每一项并处理
    for (const auto& item : value.GetArray()) {
        ProcessColumnData(item, columnDataList);
    }
}

template <typename T>
void SourceInstructionParser::ProcessColumnData(const Value& value, std::vector<T>& columnDataList)
{
    if constexpr (std::is_same<T, std::string>::value) {
        columnDataList.emplace_back(value.IsString() ? value.GetString() : "");
    } else if constexpr (std::is_same<T, int>::value) {
        columnDataList.emplace_back(value.IsInt() ? value.GetInt() : 0);
    } else if constexpr (std::is_same<T, float>::value) {
        columnDataList.emplace_back(value.IsFloat() ? value.GetFloat() : 0.0f);
    }
}

void SourceInstructionParser::ConvertApiInstr(const std::string &jsonStr)
{
    Document d;
    try {
        d.Parse(jsonStr.c_str());
        if (JsonUtil::IsJsonArray(d, "Cores")) {
            Value &cores = d["Cores"];
            for (auto &core : cores.GetArray()) {
                apiCores.emplace_back(core.GetString());
            }
        }
        // parse instructions
        if (d.HasMember("Instructions Dtype")) {
            ConvertApiInstrDynamic(jsonStr);
        }
    } catch (const std::exception &e) {
        ServerLog::Error("Can't parse api instr,not json.Error is ", e.what());
    }
}

/*
json示例
{
  "Cores": [ // 执行算子的计算核，如"core0.cubecore0"，"core0.veccore0"
    string
  ],
  "Files Dtype": { // 指定列名和数据类型
    // string 0, int 1, float 2
        "Lines": {
            "Address": 0,
            "Cycles": 1
    }
  }
  "Files": [ // 源代码文件中的代码行信息
    {
      "Lines": [ // 代码行关联的指令地址范围、消耗的时钟周期、执行指令总数
        {
          "Address Range": [ // 当前代码行关联的指令地址范围
            [
              string
            ]
          ],
          "Cycles": [ // 当前代码行在各个计算核上消耗的总时钟周期（对应顺序是？）
            int
          ],
          "Instructions Executed": [ // 当前代码行在各个计算核上执行的指令总数（对应顺序是？）
            int
          ],
          "Line": 100 // 代码行号
        }
      "Source": string // 源代码文件路径
    }
  ]
}
 */
void SourceInstructionParser::ConvertApiFileDynamic(const std::string &jsonStr)
{
    std::string errMsg;
    auto optional = JsonUtil::TryParse(jsonStr, errMsg);
    if (!optional.has_value() || !errMsg.empty()) {
        ServerLog::Error("Parse api file json failed. Error is ", errMsg);
        return;
    }
    auto &d = optional.value();
    // parse column info
    if (!d.HasMember("Files Dtype") || !d["Files Dtype"].HasMember("Lines")) {
        return;
    }
    // 遍历dtype中的列对象，获取列名和对应的数据类型
    auto &lines = d["Files Dtype"]["Lines"];
    for (auto column = lines.MemberBegin(); column != lines.MemberEnd(); ++column) {
        std::string columnName = column->name.GetString();
        auto &value = column->value;
        if (!value.IsInt()) {
            continue;
        }
        sourceLineColumnTypeMap[columnName] = value.GetInt();
    }

    if (!JsonUtil::IsJsonArray(d, "Files")) {
        return;
    }
    for (auto &file : d["Files"].GetArray()) {
        ParseFile(file);
    }
}

void SourceInstructionParser::ParseFile(Value &file)
{
    if (!JsonUtil::IsJsonArray(file, "Lines") || !file.HasMember("Source")) {
        return;
    }
    std::string sourceName = file["Source"].IsString() ? file["Source"].GetString() : "";
    if (sourceName.empty()) {
        return;
    }
    for (const auto &line: file["Lines"].GetArray()) {
        // 根据列信息动态解析Lines数据
        SourceFileLineDynamicCol sourceFileLine;
        // 这里Address Range字段不支持动态解析，需要手动解析
        ParseSourceLineAddressRange(line, sourceFileLine);
        // 根据前面获取到的列名信息，从json中解析列名对应的字段数据
        for (const auto &columnType: sourceLineColumnTypeMap) {
            std::string columnName = columnType.first;
            int type = columnType.second;
            if (!line.HasMember(columnName.c_str())) {
                continue;
            }
            // 处理不同数据类型的列
            auto &columData = line[columnName.c_str()];
            if (type == ColumDataType::STRING) {
                ProcessColumnDataArray<std::string>(columData, sourceFileLine.stringColumnMap[columnName]);
            } else if (type == ColumDataType::INT) {
                ProcessColumnDataArray<int>(columData, sourceFileLine.intColumnMap[columnName]);
            } else if (type == ColumDataType::FLOAT) {
                ProcessColumnDataArray<float>(columData, sourceFileLine.floatColumnMap[columnName]);
            }
        }

        sourceLinesMap[sourceName].emplace_back(std::move(sourceFileLine));
    }
}

void SourceInstructionParser::ParseSourceLineAddressRange(const Value &line, SourceFileLineDynamicCol &sourceFileLine)
{
    if (!line.HasMember("Address Range") || !line["Address Range"].IsArray()) {
        return;
    }
    for (auto &addressRange : line["Address Range"].GetArray()) {
        if (!addressRange.IsArray() || addressRange.Size() != addressRangeSize) {
            continue;
        }
        if (!addressRange[0].IsString() || !addressRange[1].IsString()) {
            continue;
        }
        const char *startAddress = addressRange[0].GetString();
        const char *endAddress = addressRange[1].GetString();
        sourceFileLine.addressRange.emplace_back(startAddress, endAddress);
    }
}

/*
json示例
{
  "Cores": [ // 执行算子的计算核，如"core0.cubecore0"，"core0.veccore0"
    string
  ],
  "Files": [ // 源代码文件中的代码行信息
    {
      "Lines": [ // 代码行关联的指令地址范围、消耗的时钟周期、执行指令总数
        {
          "Address Range": [ // 当前代码行关联的指令地址范围
            [
              string
            ]
          ],
          "Cycles": [ // 当前代码行在各个计算核上消耗的总时钟周期（对应顺序是？）
            int
          ],
          "Instructions Executed": [ // 当前代码行在各个计算核上执行的指令总数（对应顺序是？）
            int
          ],
          "Line": 100 // 代码行号
        }
      "Source": string // 源代码文件路径
    }
  ]
}
 */
void SourceInstructionParser::ConvertApiFile(const std::string &jsonStr)
{
    Document d;
    try {
        d.Parse(jsonStr.c_str());
        if (JsonUtil::IsJsonArray(d, "Files")) {
            Value &fileArray = d["Files"];
            apiFiles = ConvertToFileMap(fileArray);
        }
    } catch (const std::exception &e) {
        ServerLog::Error("Can't parse api file,not json.Error is ", e.what());
    }
}

std::map<std::string, std::vector<SourceFileLine>> SourceInstructionParser::ConvertToFileMap(Value &fileArray)
{
    std::map<std::string, std::vector<SourceFileLine>> sourceLinesMap;

    for (auto &file : fileArray.GetArray()) {
        if (!file.IsObject()) {
            continue;
        }

        if (!file.HasMember("Source") || !file["Source"].IsString()) {
            continue;
        }
        if (!file.HasMember("Lines") || !file["Lines"].IsArray()) {
            continue;
        }

        std::string source = file["Source"].GetString();

        rapidjson::Value &lineArray = file["Lines"];
        std::vector<SourceFileLine> sourceFileLineArray = ConvertToLineArray(lineArray);

        // 将Source和对应的SourceFileLines vector添加到map中
        sourceLinesMap[source] = sourceFileLineArray;
    }
    return sourceLinesMap;
}

std::vector<SourceFileLine> SourceInstructionParser::ConvertToLineArray(Value &lineArray)
{
    std::vector<SourceFileLine> sourceFileLines;

    for (auto &line : lineArray.GetArray()) {
        if (!line.IsObject()) {
            continue;
        }

        SourceFileLine sourceFileLine;

        // 解析Address Range数组
        if (!line.HasMember("Address Range") || !line["Address Range"].IsArray()) {
            continue;
        }
        Value &addressRangeArray = line["Address Range"];
        for (auto &addressRange : addressRangeArray.GetArray()) {
            if (!addressRange.IsArray() || addressRange.Size() != addressRangeSize) {
                continue;
            }
            if (!addressRange[0].IsString() || !addressRange[1].IsString()) {
                continue;
            }

            const char *startAddress = addressRange[0].GetString();
            const char *endAddress = addressRange[1].GetString();

            sourceFileLine.addressRange.emplace_back(startAddress, endAddress);
        }

        // 解析Cycles数组
        if (!line.HasMember("Cycles") || !line["Cycles"].IsArray()) {
            continue;
        }
        Value &cycleArray = line["Cycles"];
        for (auto &cycle : cycleArray.GetArray()) {
            sourceFileLine.cycles.emplace_back(cycle.GetFloat());
        }

        // 解析Instructions Executed数组
        if (!line.HasMember("Instructions Executed") || !line["Instructions Executed"].IsArray()) {
            continue;
        }
        Value &instrExecutedArray = line["Instructions Executed"];
        for (auto &instrExecuted : instrExecutedArray.GetArray()) {
            sourceFileLine.instructionsExecuted.emplace_back(instrExecuted.IsInt() ? instrExecuted.GetInt() : 0);
        }

        // 解析Line
        if (!line.HasMember("Line") || !line["Line"].IsInt()) {
            continue;
        }
        int lineIndex = line["Line"].GetInt();
        sourceFileLine.line = lineIndex;

        // 将解析好的SourceFileLine对象添加到vector中
        sourceFileLines.push_back(sourceFileLine);
    }
    return sourceFileLines;
}

void SourceInstructionParser::Reset()
{
    sourceFiles.clear();
    apiCores.clear();
    apiFiles.clear();
    instructionList.clear();
    instructionColumnTypeMap.clear();
    sourceLinesMap.clear();
    sourceLineColumnTypeMap.clear();
    apiInstrPos = {0, 0};
}

std::vector<std::string> SourceInstructionParser::GetCoreList()
{
    return {this->apiCores};
}

std::vector<std::string> SourceInstructionParser::GetSourceList()
{
    std::vector<std::string> sourceList;
    for (const auto &entry : sourceFiles) {
        sourceList.push_back(entry.first);
    }
    return sourceList;
}

std::vector<SourceFileLine> SourceInstructionParser::GetApiLinesByCoreAndSource(const std::string &core,
                                                                                const std::string &sourceName)
{
    std::vector<SourceFileLine> result;

    auto it = std::find(apiCores.begin(), apiCores.end(), core);
    if (it == apiCores.end()) {
        ServerLog::Error("Can't find the specified core name: ", core);
        return result;
    }
    // never below zero
    size_t index = std::distance(apiCores.begin(), it);

    if (apiFiles.find(sourceName) == apiFiles.end()) {
        ServerLog::Warn("The specified file doesn't exist in api files, and source name is ", sourceName);
        return result;
    }
    std::vector<SourceFileLine> &vector = apiFiles[sourceName];
    for (auto line : vector) {
        if (line.cycles.size() < index + 1 || line.instructionsExecuted.size() < index + 1) {
            continue;
        }

        // filter lines without instruction executed
        if (line.instructionsExecuted[index] == 0 && line.cycles[index] == 0) {
            continue;
        }

        SourceFileLine output;
        for (const auto &pair : line.addressRange) {
            output.addressRange.emplace_back(pair.first, pair.second);
        }
        output.cycles.emplace_back(line.cycles[index]);
        output.instructionsExecuted.emplace_back(line.instructionsExecuted[index]);
        output.line = line.line;

        result.emplace_back(output);
    }
    return result;
}

std::string SourceInstructionParser::GetInstr(std::string &filePath)
{
    std::ifstream file = OpenReadFileSafely(filePath, std::ios::binary);
    if (!file) {
        ServerLog::Error("Failed to open file when parse source instructions, file name is ", filePath);
        return "";
    }
    constexpr uint64_t maxDataSize = 1024 * 1024 * 200; // limit data size to 200MB
    std::string content = BinFileParseUtil::GetContentStr(file, apiInstrPos, maxDataSize);
    file.close();
    return content;
}

std::vector<SourceFileInstructionDynamicCol> SourceInstructionParser::GetInstrDynamic(std::string &coreName)
{
    std::vector<SourceFileInstructionDynamicCol> list;
    auto targetCore = std::find(apiCores.begin(), apiCores.end(), coreName);
    if (targetCore == apiCores.end()) {
        targetCore = apiCores.begin();
    }
    size_t index = std::distance(apiCores.begin(), targetCore); // never below zero

    for (const auto &item: instructionList) {
        SourceFileInstructionDynamicCol col;
        GetInstrValueInTargetCore(item.intColumnMap, col.intColumnMap, index);
        GetInstrValueInTargetCore(item.floatColumnMap, col.floatColumnMap, index);
        GetInstrValueInTargetCore(item.stringColumnMap, col.stringColumnMap, index);
        list.emplace_back(col);
    }

    return list;
}

template<typename T>
void SourceInstructionParser::GetInstrValueInTargetCore(
    const std::unordered_map<std::string, std::vector<T>> &sourceMap,
    std::unordered_map<std::string, std::vector<T>> &targetMap,
    size_t index)
{
    for (const auto &sourceItem: sourceMap) {
        if (sourceItem.second.empty() || sourceItem.second.size() <= index) {
            continue;
        }
        // 如果是数组，获取指定core上的数据，否则取第一个值
        if (sourceItem.second.size() == 1) {
            targetMap[sourceItem.first].emplace_back(sourceItem.second[0]);
            continue;
        }
        targetMap[sourceItem.first].emplace_back(sourceItem.second[index]);
    }
}

std::string SourceInstructionParser::GetSourceByName(std::string &sourceName, std::string &filePath)
{
    if (sourceFiles.count(sourceName) == 0) {
        ServerLog::Warn("Don't exist the specified file ", sourceName);
        return "";
    }
    Position &pos = sourceFiles[sourceName];

    std::ifstream file = OpenReadFileSafely(filePath, std::ios::binary);
    if (!file) {
        ServerLog::Error("Failed to open file when get source code by name, file name is ", filePath);
        return "";
    }

    std::string content = BinFileParseUtil::GetContentStr(file, pos);
    file.close();
    return content;
}

std::map<std::string, int> SourceInstructionParser::GetInstructionColumnTypeMap() const
{
    return instructionColumnTypeMap;
}

std::map<std::string, int> SourceInstructionParser::GetSourceLineColumnTypeMap() const
{
    return sourceLineColumnTypeMap;
}

} // Dic
} // Module
} // Source