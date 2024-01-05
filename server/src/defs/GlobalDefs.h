/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: defines declaration
 */

#ifndef DATA_INSIGHT_CORE_DEFS_H
#define DATA_INSIGHT_CORE_DEFS_H

#include <json.hpp>
#include "rapidjson.h"
#include "document.h"
#include "writer.h"
#include "stringbuffer.h"

namespace Dic {
using json_t = nlohmann::json;
using namespace rapidjson;
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_DEFS_H