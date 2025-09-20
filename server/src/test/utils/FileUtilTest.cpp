/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include <gtest/gtest.h>
#include "FileUtil.h"
#include "IdBuilder.h"
#include "../TestSuit.cpp"
#include "JsonUtil.h"

using namespace Dic;

class FileUtilTest : public  TestSuit {
};

TEST_F(TestSuit, BasicAssertions)
{
#ifdef _WIN32
    EXPECT_EQ(FileUtil::SplicePath("a", "b"), "a\\b");
    EXPECT_EQ(FileUtil::SplicePath("a", "b", "c"), "a\\b\\c");
#else
    EXPECT_EQ(FileUtil::SplicePath("a", "b"), "a/b");
#endif
}

TEST(TestUtil, testGetDouble)
{
    json_t json;
    rapidjson::Document d;
    d.Parse("{\n"
            "        \"ph\": \"X\",\n"
            "        \"name\": \"contiguous_d_Reshape\",\n"
            "        \"pid\": 768209,\n"
            "        \"tid\": 768209,\n"
            "        \"ts\": \"1699579270364817.47\",\n"
            "        \"dur\": \"169.33\",\n"
            "        \"cat\": \"cpu_op\",\n"
            "        \"args\": {}\n"
            "}");
    d.GetAllocator();
    double ts = JsonUtil::GetDouble(d, "ts");
    double dur = JsonUtil::GetDouble(d, "dur");
    EXPECT_EQ(ts, 1699579270364817.47);
    EXPECT_EQ(dur, 169.33);
}

TEST(TestUtil, TestSplitToRankList)
{
    std::string currPath = Dic::FileUtil::GetCurrPath();
    int index = currPath.find_last_of("server");
    currPath = currPath.substr(0, index + 1);
    std::vector<std::pair<std::string, std::string>> fileList;
    std::pair<std::string, std::string> pair1;
    pair1.first = "1";
    pair1.second = currPath + "/src/test/test_data/test_rank_1/ASCEND_PROFILER_OUTPUT/trace_view.json";
    fileList.push_back(pair1);
    std::pair<std::string, std::string> pair2;
    pair1.first = "0";
    pair1.second = currPath + "/src/test/test_data/test_rank_0/ASCEND_PROFILER_OUTPUT/trace_view.json";
    fileList.push_back(pair2);
    std::map<std::string, std::vector<std::string>> result = FileUtil::SplitToRankList(fileList);
    EXPECT_EQ(result.size(), 2);
}

TEST(TestUtil, TestGetRankIdFromFile)
{
    std::string currPath = Dic::FileUtil::GetCurrPath();
    int index = currPath.find_last_of("server");
    currPath = currPath.substr(0, index + 1);
    std::string rank = FileUtil::GetRankIdFromFile(
            currPath + "/src/test/test_data/test_rank_1/ASCEND_PROFILER_OUTPUT/trace_view.json");
    EXPECT_EQ(rank, "1");
}

TEST(TestUtil, TestGetRankIdFromPath)
{
    std::string currPath = Dic::FileUtil::GetCurrPath();
    int index = currPath.find_last_of("server");
    currPath = currPath.substr(0, index + 1);
    std::string rank = FileUtil::GetRankIdFromPath(
            currPath + "/src/test/test_data/test_rank_1/ASCEND_PROFILER_OUTPUT/trace_view.json");
    bool result = FileUtil::CheckFilePath(
            currPath + "/src/test/test_data/test_rank_1/ASCEND_PROFILER_OUTPUT/trace_view.json");
    EXPECT_EQ(rank, "test_rank_1");
    EXPECT_EQ(result, true);
}

TEST(TestUtil, TestGetDbPath)
{
    std::string currPath = Dic::FileUtil::GetCurrPath();
    int index = currPath.find_last_of("server");
    currPath = currPath.substr(0, index + 1);
#ifdef _WIN32
    std::string fileId = FileUtil::GetDbPath(
            currPath + "\\src\\test\\test_data\\test_rank_1\\ASCEND_PROFILER_OUTPUT\\trace_view.json", "1");
    EXPECT_EQ(fileId,
            currPath + "\\src\\test\\test_data\\test_rank_1\\ASCEND_PROFILER_OUTPUT\\mindstudio_insight_data.db");
#else
    std::string dbPath = FileUtil::GetDbPath(
            currPath + "/src/test/test_data/test_rank_1/ASCEND_PROFILER_OUTPUT/trace_view.json", "1");
    EXPECT_EQ(dbPath, currPath + "/src/test/test_data/test_rank_1/ASCEND_PROFILER_OUTPUT/mindstudio_insight_data.db");
#endif
}

TEST(TestUtil, TestIdBuilder)
{
    int id2 = IdBuilder::EventIdBuilder().Build();
    int id3 = IdBuilder::RequestIdBuilder().Build();
    int id4 = IdBuilder::SessionIdBuilder().Build();
    EXPECT_EQ(id2, 0);
    EXPECT_EQ(id3, 0);
    EXPECT_EQ(id4, 0);
}

TEST(TestUtil, GetFileId)
{
    EXPECT_EQ(FileUtil::GetSingleFileIdWithDb("test"), "test_mindstudio_insight_data.db");
}

TEST(TestUtil, TestGetFileSizeNullFileName)
{
    auto res = FileUtil::GetFileSize(nullptr);
    EXPECT_EQ(res, 0);
}

TEST(TestUtil, TestIsAbsolutePathEmtpyPath)
{
    EXPECT_EQ(FileUtil::IsAbsolutePath(""), false);
}

TEST(TestUtil, CheckDirAccessSuccessWhenFileExist)
{
    std::ofstream file(".//example.txt");
    file.close();
    EXPECT_EQ(FileUtil::CheckDirAccess(".//example.txt"), true);
    EXPECT_EQ(std::remove(".//example.txt"), 0);
}
 
TEST(TestUtil, CheckDirAccessFailedWhenFileIsNotExist)
{
    EXPECT_EQ(FileUtil::CheckDirAccess("./test1.text"), false);
}
 
TEST(TestUtil, CheckFilePathLengthFailedWhenFilePathIsTooLong)
{
#ifdef _WIN32
    std::string filePath(MAX_PATH, 'a');
    EXPECT_EQ(FileUtil::CheckFilePathLength(filePath), false);
#else
    std::string filePath(PATH_MAX, 'a');
    EXPECT_EQ(FileUtil::CheckFilePathLength(filePath), false);
#endif
}
 
TEST(TestUtil, CheckFilePathLengthSuccess)
{
#ifdef _WIN32
    std::string filePath("test11");
    EXPECT_EQ(FileUtil::CheckFilePathLength(filePath), true);
#else
    std::string filePath("test111");
    EXPECT_EQ(FileUtil::CheckFilePathLength(filePath), true);
#endif
}
 
TEST(TestUtil, CheckFilePathExistSuccessWhenFileIsExist)
{
    std::ofstream file(".//example.txt");
    file.close();
    EXPECT_EQ(FileUtil::CheckFilePathExist(".//example.txt"), true);
    EXPECT_EQ(std::remove(".//example.txt"), 0);
}
 
TEST(TestUtil, CheckFilePathExistFailedWhenFileIsNotExist)
{
    EXPECT_EQ(FileUtil::CheckFilePathExist(".//example_no_exist.txt"), false);
}
 
TEST(TestUtil, IsAbsolutePathFailedWhenPathIsRelativePath)
{
#ifdef _WIN32
    EXPECT_EQ(FileUtil::IsAbsolutePath("\\dbox\\example_no_exist.txt"), false);
    EXPECT_EQ(FileUtil::IsAbsolutePath("a"), false);
#else
    EXPECT_EQ(FileUtil::IsAbsolutePath("./home/test"), false);
#endif
}
 
TEST(TestUtil, IsAbsolutePathCheckSuccessWhenPathIsAbsPath)
{
#ifdef _WIN32
    EXPECT_EQ(FileUtil::IsAbsolutePath("D:\\dbox\\example_no_exist.txt"), true);
#else
    EXPECT_EQ(FileUtil::IsAbsolutePath("/root/home/test"), true);
#endif
}
 
TEST(TestUtil, GetAbsPathFailedWhenPathIsEmpty)
{
    EXPECT_EQ(FileUtil::GetAbsPath(""), "");
}
 
TEST(TestUtil, GetAbsPathSuccessWhenPathIsExist)
{
    std::ofstream file(".//example.txt");
    file.close();
    EXPECT_NE(FileUtil::GetAbsPath(".//example.txt"), "");
    EXPECT_EQ(std::remove(".//example.txt"), 0);
}
 
TEST(TestUtil, IsSoftLinkCheckFailedWhenPathIsExistAndIsNotSoftlink)
{
    std::ofstream file(".//example.txt");
    file.close();
    EXPECT_EQ(FileUtil::IsSoftLink(".//example.txt"), false);
    EXPECT_EQ(std::remove(".//example.txt"), 0);
}
 
TEST(TestUtil, IsSoftLinkCheckFailedWhenPathIsNotExist)
{
    EXPECT_EQ(FileUtil::IsSoftLink(".//example_bot_exist.txt"), false);
}
 
TEST(TestUtil, IsSoftLinkCheckSuccessWhenPathIsSoftlink)
{
    // 源文件路径
    const char* srcPath = ".//example.txt";
    // 链接文件路径
    const char* linkPath = ".//example_softlink.txt";
    std::ofstream file(srcPath);
    file.close();
#ifdef _WIN32
    // 创建软链接
    DWORD flags = SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
    EXPECT_NE(CreateSymbolicLink(linkPath, srcPath, flags), 0);
    EXPECT_EQ(FileUtil::IsSoftLink(linkPath), true);
#else
    // 创建软链接
    EXPECT_EQ(symlink(srcPath, linkPath), 0);
    EXPECT_EQ(FileUtil::IsSoftLink(".//example_softlink.txt"), true);
#endif
    EXPECT_NE(unlink(linkPath), -1);
    EXPECT_EQ(std::remove(srcPath), 0);
}
 
TEST(TestUtil, CheckPathValidFailedWhenPathIsNotExist)
{
    EXPECT_EQ(FileUtil::CheckDirValid(".//example_bot_exist.txt"), false);
}
 
TEST_F(TestSuit, CheckPathValidFailedWhenPathIsEmpty)
{
    EXPECT_EQ(FileUtil::CheckDirValid(""), false);
}
 
TEST(TestUtil, CheckPathValidFailedWhenFileIsExistedButPathIsTooLong)
{
#ifdef _WIN32
    std::string filePath(MAX_PATH, 'a');
    EXPECT_EQ(FileUtil::CheckDirValid(filePath), false);
#else
    std::string filePath(PATH_MAX, 'a');
    EXPECT_EQ(FileUtil::CheckDirValid(filePath), false);
#endif
}
 
TEST(TestUtil, CheckPathValidFailedWhenFileExistInvalidChar)
{
    EXPECT_EQ(FileUtil::CheckDirValid("te\\nst.text"), false);
}
 
TEST(TestUtil, CheckPathValidFailedWhenFileIsSoftlink)
{
    // 源文件路径
    const char* srcPath = ".//example.txt";
    // 链接文件路径
    const char* linkPath = ".//example_softlink.txt";
    std::ofstream file(srcPath);
    file.close();
#ifdef _WIN32
    // 创建软链接
    DWORD flags = SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
    EXPECT_NE(CreateSymbolicLink(linkPath, srcPath, flags), 0);
    EXPECT_EQ(FileUtil::CheckDirValid("te\\nst.text"), false);
#else
    // 创建软链接
    EXPECT_EQ(symlink(srcPath, linkPath), 0);
    EXPECT_EQ(FileUtil::CheckDirValid("te\\nst.text"), false);
#endif
    EXPECT_NE(unlink(linkPath), -1);
    EXPECT_EQ(std::remove(srcPath), 0);
}
 
TEST(TestUtil, CheckPathValidSuccessWhenFileIsExist)
{
    // 源文件路径
    const char* srcPath = ".//example.txt";
    std::ofstream file(srcPath);
    file.close();
    EXPECT_EQ(FileUtil::CheckDirValid(srcPath), true);
    EXPECT_EQ(std::remove(srcPath), 0);
}
 
TEST(TestUtil, CheckPathValidSuccessWhenFileIsExistAndPathIsInChinese)
{
#ifdef _WIN32
    const wchar_t* filePath = L".\\测试001.txt";
    HANDLE hFile = CreateFileW(filePath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "无法创建文件: " << GetLastError() << std::endl;
    }
    CloseHandle(hFile);
    EXPECT_EQ(FileUtil::CheckDirValid(".\\测试001.txt"), true);
    EXPECT_NE(DeleteFileW(filePath), 0);
#else
    // 源文件路径
    const char* srcPath = ".//测试001.txt";
    std::ofstream file(srcPath);
    file.close();
    EXPECT_EQ(FileUtil::CheckDirValid(srcPath), true);
    EXPECT_EQ(std::remove(srcPath), 0);
#endif
}
 
TEST(TestUtil, CheckFileSizeSuccessWhenFileIsEmptyAndPathIsInChinese)
{
#ifdef _WIN32
    const wchar_t* filePath = L".\\测试001.txt";
    HANDLE hFile = CreateFileW(filePath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "无法创建文件: " << GetLastError() << std::endl;
    }
    // 写入数据
    const char* data = "这是第一行\n这是第二行\n这是第三行\n";
    DWORD bytesWritten;
    if (!WriteFile(hFile, data, strlen(data), &bytesWritten, NULL)) {
        std::cerr << "无法写入文件: " << GetLastError() << std::endl;
        CloseHandle(hFile);
    }
    CloseHandle(hFile);
    EXPECT_EQ(FileUtil::CheckFileSize(".\\测试001.txt"), true);
    EXPECT_NE(DeleteFileW(filePath), 0);
#else
    // 源文件路径
    const char* srcPath = ".//测试001.txt";
    std::ofstream file(srcPath);
    if (file.is_open()) {
        file << "测试1" << std::endl;
        file << "测试2" << std::endl;
        file.close();
    }
    EXPECT_EQ(FileUtil::CheckDirValid(srcPath), true);
    EXPECT_EQ(std::remove(srcPath), 0);
#endif
}
 
TEST(TestUtil, CheckFileSizeFailedWhenFileIsExistButIsEmpty)
{
    // 源文件路径
    const char* srcPath = ".//example.txt";
    std::ofstream file(srcPath);
    file.close();
    EXPECT_EQ(FileUtil::CheckFileSize(srcPath), false);
    EXPECT_EQ(std::remove(srcPath), 0);
}

TEST(TestUtil, GetRealPathSuccessWhenFileIsExistAndIsNotEmpty)
{
    // 源文件路径
    const char* srcPath = ".//example.txt";
    std::ofstream file(srcPath);
    file.close();
    EXPECT_NE(FileUtil::GetRealPath(srcPath), "");
    EXPECT_EQ(std::remove(srcPath), 0);
}

TEST(TestUtil, GetRealPathFailedWhenFileIsNotExist)
{
#ifdef _WIN32
    std::string filePath = "D:\\test\\test1\\example.txt";
    EXPECT_EQ(FileUtil::GetRealPath(filePath), filePath);
#else
    std::string filePath = "D://test/test1/example.txt";
    EXPECT_EQ(FileUtil::GetRealPath(filePath), "");
#endif
}

TEST(TestUtil, ConvertToRealPath)
{
#ifdef  _WIN32
    return;
#else
    std::vector<std::string> paths = {""};
    std::string errMsg;
    bool suc = FileUtil::ConvertToRealPath(errMsg, paths);
    EXPECT_EQ(suc, false);
    EXPECT_EQ(errMsg, "is invalid path");
    paths[0] = "/etc/hosts";
    errMsg.clear();
    EXPECT_EQ(FileUtil::ConvertToRealPath(errMsg, paths), true);
    EXPECT_EQ(errMsg.empty(), true);
#endif
}

TEST(TestUitl, GetRelativePath)
{
    std::string path1 = "/etc/host/test";
    std::string path2 = "/etc/host";
    auto res = FileUtil::GetRelativePath(path1, path2);
    EXPECT_NE(res, nullptr);
    EXPECT_EQ(res->compare("test"), 0);
    res = FileUtil::GetRelativePath(path2, path1);
    EXPECT_EQ(res, nullptr);
}

TEST(TestUtil, GetRootPath)
{
    std::string path = "/etc/hosts";
    EXPECT_EQ(FileUtil::GetRootPath(path), "/");
    path = "hosts";
    EXPECT_EQ(FileUtil::GetRootPath(path), "");
}

TEST(TestUtil, FindIfDbTypeByRegex)
{
    std::string currPath = Dic::FileUtil::GetCurrPath();
    int index = currPath.find_last_of("server");
    currPath = currPath.substr(0, index + 1);
    auto testDbDir = currPath + R"(/src/test/test_data/full_db)";
    const std::string DB_REG =
            R"((msprof_[0-9]{1,16}|((ascend_pytorch_profiler)(_[0-9]{1,16}){0,1})|cluster_analysis)\.db$)";
    const std::string traceViewReg =
            R"((((trace_view|msprof(_slice_[0-9]{1,2})?_[0-9]{1,14})\.json)|)"
            R"((operator_memory|operator_memory(_slice_[0-9]{1,2})?_[0-9]{1,14})\.csv)$)";
    bool suc = FileUtil::FindIfDbTypeByRegex(testDbDir, std::regex(traceViewReg), std::regex(DB_REG));
    EXPECT_EQ(suc, true);
}

TEST(TestUtil, CopyFileByPath)
{
    std::string sourcePath = "CopyFileByPathTest.tmp";
    std::ofstream sourceFile(sourcePath);
    EXPECT_TRUE(sourceFile.is_open());
    sourceFile.close();
    std::string targetPath = "./CopyFileByPathTest.copy";
    EXPECT_TRUE(FileUtil::CopyFileByPath(sourcePath, targetPath));

    // remove file
    std::remove(sourcePath.c_str());
    std::remove(targetPath.c_str());
}

TEST(TestUtil, TestSplitFilePathSuccess)
{
#ifdef __WIN32
    std::string dbPath1 = R"(D:\GUI_TEST_DATA\deepseek_32B\actor worker\ma-job_ascend_pt\ASCEND_PROFILER_OUTPUT\)";
#else
    std::string dbPath1 = "D:/GUI_TEST_DATA/deepseek_32B/actor worker/ma-job_ascend_pt/ASCEND_PROFILER_OUTPUT/";
#endif
    auto result1 = FileUtil::SplitFilePath(dbPath1);
    std::vector<std::string> expected1 = { "D:", "GUI_TEST_DATA", "deepseek_32B",
        "actor worker", "ma-job_ascend_pt", "ASCEND_PROFILER_OUTPUT" };
    EXPECT_EQ(result1, expected1);
}

TEST(TestUtil, SplicePath)
{
    EXPECT_EQ("/home/user/test", FileUtil::SplicePath("/home", "user", "test"));
}

TEST(TestUtil, StemFile)
{
    EXPECT_EQ(FileUtil::StemFile("test.excel"), "test");
    EXPECT_EQ(FileUtil::StemFile("/home/user/test.tar"), "test");
    EXPECT_EQ(FileUtil::StemFile("test_excel"), "test_excel");
    EXPECT_EQ(FileUtil::StemFile("test.tar.gz"), "test");
}