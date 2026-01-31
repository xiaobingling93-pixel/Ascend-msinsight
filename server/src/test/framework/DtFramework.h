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

#ifndef PROFILER_SERVER_DTFRAMEWORK_H
#define PROFILER_SERVER_DTFRAMEWORK_H
#include <string>
namespace Dic::DT::Framework
{
/**
 * @brief  This class used to provide public func to DT write
 */
class DtFramework
{
public:
    /**
     *
     * @param version 0- {projectRootPath}/server/src/test/test_data,  1-{projectRootPath}/test
     * @return string
     */
    static std::string GetTestDataDirPath(int version=0);

};
}

#endif //PROFILER_SERVER_DTFRAMEWORK_H
