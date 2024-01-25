/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#include <string>
#include "ServerLog.h"
#include "TokenBuilder.h"

#ifdef _WIN32
#include <wincrypt.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

namespace Dic {
namespace Server {

TokenBuilder &TokenBuilder::Instance()
{
    static TokenBuilder instance;
    return instance;
}

const std::string TokenBuilder::Build()
{
    static const int TOKEN_LENGTH = 16;
    static const int CHAR_SIZE = 26;

#ifdef _WIN32
    HCRYPTPROV hCryptProv;
    BYTE pbData[TOKEN_LENGTH];
    if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        ServerLog::Error("Error acquiring cryptographic context.");
        return "";
    }
    if (!CryptGenRandom(hCryptProv, TOKEN_LENGTH, pbData)) {
        CryptReleaseContext(hCryptProv, 0);
        ServerLog::Error("Error generating random data.");
        return "";
    }
    CryptReleaseContext(hCryptProv, 0);

    std::string tokenString;
    tokenString.reserve(TOKEN_LENGTH);
    std::copy(pbData, pbData + TOKEN_LENGTH, std::back_inserter(tokenString));
    for (size_t i = 0; i < TOKEN_LENGTH; ++i) {
        tokenString[i] = static_cast<char>((tokenString[i] % CHAR_SIZE) + static_cast<int>('a'));  // Convert to a-z
    }
    return tokenString;
#else
    std::string tokenString;
    tokenString.resize(TOKEN_LENGTH);

    int fd = open("/dev/random", O_RDONLY);
    if (fd < 0) {
        ServerLog::Error("Error opening random file.");
        return "";
    }
    if (read(fd, &tokenString[0], TOKEN_LENGTH) != TOKEN_LENGTH) {
        close(fd);
        ServerLog::Error("Error reading from random file.");
        return "";
    }
    close(fd);
    for (size_t i = 0; i < TOKEN_LENGTH; ++i) {
        tokenString[i] = static_cast<char>((tokenString[i] % CHAR_SIZE) + static_cast<int>('a'));  // Convert to a-z
    }
    return tokenString;
#endif
}
} // end of namespace Server
} // end of namespace Dic