/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_SOCKET_UTIL_H
#define PROFILER_SERVER_SOCKET_UTIL_H

#include <cstdio>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

namespace Dic {
class SocketUtil {
public:
    static bool PortIsUsed(int port)
    {
#ifdef _WIN32
        WSADATA wsaData;
        static const int VERSION_HIGH = 2;
        static const int VERSION_LOW = 2;
        if (WSAStartup(MAKEWORD(VERSION_HIGH, VERSION_LOW), &wsaData) != 0) {
            return false;
        }
#endif
        auto soc = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        sockaddr_in sin{};
        sin.sin_family = AF_INET;
        sin.sin_port = htons(port);
#ifdef _WIN32
        sin.sin_addr.S_un.S_addr = INADDR_ANY;
        int flag = 1;
        setsockopt(soc, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (const char*)&flag, sizeof(flag));
        auto res = bind(soc, (struct sockaddr*)&sin, sizeof(sin));
        closesocket(soc);
        WSACleanup();
#else
        sin.sin_addr.s_addr = INADDR_ANY;
        auto res = bind(soc, (struct sockaddr*)&sin, sizeof(sin));
        close(soc);
#endif
        return res == -1;
    }
};
} // end of namespace Dic
#endif // PROFILER_SERVER_SOCKET_UTIL_H