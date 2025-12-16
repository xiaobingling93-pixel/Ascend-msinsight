#if (defined(_WIN32) || defined(_WIN64) || defined(_MSC_VER))
#define MSPTI_API __declspec(dllexport)
#else
#define MSPTI_API __attribute__((visibility("default")))
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "mspti.h"

void UserBufferRequest(uint8_t **buffer, size_t *size, size_t *maxNumRecords)
{
    if (buffer == nullptr || size == nullptr || maxNumRecords == nullptr) {
        return;
    }
    *maxNumRecords = 0;
    auto bufferSize = 4*1024*1024;
    auto *pBuffer = (uint8_t *)malloc(bufferSize); 
    if (pBuffer == nullptr) {
        *buffer = nullptr;
        *size = 0;
    } else {
        *buffer = pBuffer;
        *size = bufferSize;
    }
}

void UserBufferConsume(msptiActivity *record)
{
    if (record == nullptr) {
        return;
    }
    if (record->kind == MSPTI_ACTIVITY_KIND_COMMUNICATION) {
        msptiActivityCommunication *data = (msptiActivityCommunication*)(record);
        fprintf(stderr, "[Communication] kind: %d, dataType: %d, count: %lu, deviceId: %u, streamId: %u, start: %lu, end: %lu, algType: %s, name: %s, commName: %s, correlationId: %lu\n", 
            data->kind, data->dataType, data->count, data->ds.deviceId, data->ds.streamId, data->start, data->end, data->algType, data->name, data->commName, data->correlationId);
    } else if(record->kind == MSPTI_ACTIVITY_KIND_API) {
        msptiActivityApi *api = reinterpret_cast<msptiActivityApi*>(record);
        fprintf(stderr, "[Api] kind: %d, name: %s, start: %lu, end: %lu, processId: %u, threadId: %u, correlationId: %lu\n",
                api->kind, api->name, api->start, api->end, api->pt.processId, api->pt.threadId,
                api->correlationId);
    }
}

void UserBufferComplete(uint8_t *buffer, size_t size, size_t validSize)
{
    if (validSize > 0 && buffer != nullptr) {
        msptiActivity *record = nullptr;
        msptiResult status = MSPTI_SUCCESS;
        do {
            status = msptiActivityGetNextRecord(buffer, validSize, &record);
            if (status == MSPTI_SUCCESS) {
                UserBufferConsume(record);
            } else {
                break;
            }
        } while (true);
    }
    free(buffer);
}

extern "C" {
msptiSubscriberHandle subscriber;
MSPTI_API void MsptiStart()
{
    msptiSubscribe(&subscriber, nullptr, nullptr);
    msptiActivityRegisterCallbacks(UserBufferRequest, UserBufferComplete);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_COMMUNICATION);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_API);
}

MSPTI_API void MsptiStop()
{
    msptiUnsubscribe(subscriber);
}

MSPTI_API void MsptiFlushAll()
{
    msptiActivityFlushAll(1);
}
}
