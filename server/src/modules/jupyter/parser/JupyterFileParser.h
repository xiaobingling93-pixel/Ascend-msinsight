/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_JUPYTERFILEPARSER_H
#define PROFILER_SERVER_JUPYTERFILEPARSER_H

#include "ThreadPool.h"

namespace Dic {
namespace Module {
namespace Jupyter {
class JupyterFileParser {
public:
    static JupyterFileParser &Instance();
    std::shared_ptr<ThreadPool> GetThreadPool();
private:
    JupyterFileParser();
    ~JupyterFileParser();
    std::shared_ptr<ThreadPool> singleThreadPool;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_JUPYTERFILEPARSER_H
