/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#ifndef MSINSIGHT_APIHANDLER_H
#define MSINSIGHT_APIHANDLER_H
#include "string"
namespace Dic::Core {
enum class API_TYPE {
    GET,
    POST
};
class ApiHandler {
public:
    ApiHandler(API_TYPE type): apiType_(type) {};
    virtual ~ApiHandler() = default;
    virtual bool run(std::string_view data, std::string &result) = 0;
    API_TYPE GetApiType() { return apiType_; };
protected:
    API_TYPE apiType_;
};
class GetHandler : public ApiHandler {
public:
    GetHandler(): ApiHandler(API_TYPE::GET) {};
    virtual ~GetHandler() = default;
    virtual bool run(std::string_view data, std::string &result) = 0;
};
class PostHandler : public ApiHandler {
public:
    PostHandler(): ApiHandler(API_TYPE::POST) {};
    virtual ~PostHandler() = default;
    virtual bool run(std::string_view data, std::string &result) = 0;
};
}
#endif // MSINSIGHT_APIHANDLER_H