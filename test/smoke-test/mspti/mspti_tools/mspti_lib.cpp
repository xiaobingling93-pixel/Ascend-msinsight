#if (defined(_WIN32) || defined(_WIN64) || defined(_MSC_VER))
#define MSPTI_API __declspec(dllexport)
#else
#define MSPTI_API __attribute__((visibility("default")))
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "mspti.h"
#include "common/helper_mspti.h"

extern "C" {
MSPTI_API void MsptiStart()
{
    msptiSubscribe(&subscriber, nullptr, nullptr);
    msptiActivityRegisterCallbacks(UserBufferRequest, UserBufferComplete);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_MARKER);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_KERNEL);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_API);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_COMMUNICATION);
}

MSPTI_API void MsptiStop()
{
    msptiUnsubscribe(subscriber);
    msptiActivityFlushAll(1);
}

MSPTI_API void MsptiFlushAll()
{
    msptiActivityFlushAll(1);
}
}
