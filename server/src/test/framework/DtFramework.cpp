// /*
//  * -------------------------------------------------------------------------
//  * This file is part of the MindStudio project.
//  * Copyright (c)  2026 Huawei Technologies Co.,Ltd.
//  *
//  * MindStudio is licensed under Mulan PSL v2.
//  * You can use this software according to the terms and conditions of the Mulan PSL v2.
//  * You may obtain a copy of Mulan PSL v2 at:
//  *
//  *          http://license.coscl.org.cn/MulanPSL2
//  *
//  * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
//  * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
//  * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
//  * See the Mulan PSL v2 for more details.
//  * -------------------------------------------------------------------------
//  *

#include "DtFramework.h"
#include "FileUtil.h"
#include "StringUtil.h"
namespace Dic::DT::Framework
{
std::string DtFramework::GetTestDataDirPath(int version)
{
    std::string currentPath = FileUtil::GetCurrPath();
    auto pos = currentPath.find("server");
    auto prefix = currentPath.substr(0, pos + sizeof("server"));
    if (version == 0) {
        return FileUtil::SplicePath(prefix, "src", "test", "test_data");
    }
    prefix = FileUtil::GetParentPath(prefix);
    return FileUtil::SplicePath(prefix, "test");
}
}
